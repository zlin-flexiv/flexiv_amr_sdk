/**
 * @example seer_audio_example.cpp
 * This example uploads, downloads, plays, or pauses audio on a SEER AMR.
 */

#include <algorithm>
#include <chrono>
#include <cctype>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include <flexiv/amr/exceptions.h>
#include <flexiv/amr/vendor/seer/seer_amr.h>
#include <spdlog/spdlog.h>

namespace {

struct ProgramOptions {
    std::string amr_ip;
    std::string audio_name;
    std::string upload_file;
    std::string download_file;
    bool loop = false;
    bool pause = false;
};

constexpr auto kUploadReadyDelay = std::chrono::seconds(1);
constexpr auto kStatusPollInterval = std::chrono::milliseconds(300);
constexpr int kStatusPollCount = 8;

void PrintUsage(const char* program) {
    std::cerr
        << "Usage:\n"
        << "  " << program
        << " <amr_ip> [audio_name|audio_file] [--loop] [--download output_file] [--pause]\n\n"
        << "Arguments:\n"
        << "  audio_name  Audio name stored on the AMR, without file extension.\n"
        << "  audio_file  Local .wav file. If provided, upload it before playback.\n\n"
        << "Examples:\n"
        << "  " << program << " 192.168.192.5 welcome --loop\n"
        << "  " << program << " 192.168.192.5 ./welcome.wav\n"
        << "  " << program << " 192.168.192.5 welcome --download ./welcome.wav\n"
        << "  " << program << " 192.168.192.5 --pause\n";
}

std::string ToLower(std::string text) {
    std::transform(text.begin(), text.end(), text.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return text;
}

bool IsAudioFilePath(const std::string& path) {
    const auto extension = ToLower(std::filesystem::path(path).extension().string());
    return extension == ".wav";
}

bool HasUnsupportedAudioExtension(const std::string& path) {
    const auto extension = ToLower(std::filesystem::path(path).extension().string());
    return extension == ".mp3";
}

std::string AudioNameFromPath(const std::string& path) {
    const auto stem = std::filesystem::path(path).stem().string();
    if (stem.empty()) {
        throw std::invalid_argument("audio file path must contain a file name");
    }
    return stem;
}

ProgramOptions ParseArguments(int argc, char* argv[]) {
    if (argc < 2) {
        throw std::invalid_argument("missing required arguments");
    }

    ProgramOptions options;
    options.amr_ip = argv[1];

    for (int i = 2; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--loop") {
            options.loop = true;
        } else if (arg == "--pause") {
            options.pause = true;
        } else if (arg == "--download") {
            if (i + 1 >= argc) {
                throw std::invalid_argument("--download requires an output file path");
            }
            options.download_file = argv[++i];
        } else if (HasUnsupportedAudioExtension(arg)) {
            throw std::invalid_argument("only .wav audio files are supported");
        } else if (IsAudioFilePath(arg) && options.upload_file.empty()) {
            options.upload_file = arg;
        } else if (options.audio_name.empty()) {
            options.audio_name = arg;
        } else {
            throw std::invalid_argument("unexpected argument: " + arg);
        }
    }

    if (!options.upload_file.empty()) {
        options.audio_name = AudioNameFromPath(options.upload_file);
    }
    if (!options.pause && options.audio_name.empty()) {
        throw std::invalid_argument("audio_name is required unless --pause is used alone");
    }
    if (!options.download_file.empty() && options.audio_name.empty()) {
        throw std::invalid_argument("audio_name is required when downloading audio");
    }

    return options;
}

void WriteBinaryFile(const std::string& path, const std::vector<uint8_t>& data) {
    std::ofstream file(path, std::ios::binary);
    if (!file) {
        throw std::runtime_error("failed to open output file: " + path);
    }

    file.write(reinterpret_cast<const char*>(data.data()),
               static_cast<std::streamsize>(data.size()));
    if (!file) {
        throw std::runtime_error("failed to write output file: " + path);
    }
}

std::vector<uint8_t> ReadBinaryFile(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        throw std::runtime_error("failed to open input file: " + path);
    }

    return std::vector<uint8_t>((std::istreambuf_iterator<char>(file)),
                                std::istreambuf_iterator<char>());
}

flexiv::amr::seer::SeerAudioStatus LogAudioStatus(flexiv::amr::seer::SeerAmrClient& amr,
                                                  const char* context) {
    const auto status = amr.GetAudioStatus();
    spdlog::info("{} audio status: status={}, sound_name='{}', loop={}, count={}",
                 context,
                 status.status,
                 status.sound_name,
                 status.loop,
                 status.count);
    return status;
}

void VerifyUploadedAudio(flexiv::amr::seer::SeerAmrClient& amr,
                         const std::string& audio_name,
                         const std::string& upload_file) {
    spdlog::info("Waiting {} ms for the AMR to index uploaded audio ...",
                 std::chrono::duration_cast<std::chrono::milliseconds>(kUploadReadyDelay).count());
    std::this_thread::sleep_for(kUploadReadyDelay);

    const auto local_data = ReadBinaryFile(upload_file);
    spdlog::info("Verifying upload by downloading audio '{}' ...", audio_name);
    const auto downloaded_data = amr.DownloadAudio(audio_name);

    if (downloaded_data.size() != local_data.size()) {
        spdlog::warn("Downloaded size {} differs from local size {}. The upload was readable by "
                     "name, but the bytes are not identical.",
                     downloaded_data.size(),
                     local_data.size());
        return;
    }

    if (downloaded_data != local_data) {
        spdlog::warn("Downloaded audio '{}' has the same size as the local file but different "
                     "content.",
                     audio_name);
        return;
    }

    spdlog::info("Upload verification succeeded: '{}' is readable by name and bytes match.",
                 audio_name);
}

void PollAudioStatusAfterPlay(flexiv::amr::seer::SeerAmrClient& amr,
                              const std::string& audio_name) {
    for (int i = 0; i < kStatusPollCount; ++i) {
        std::this_thread::sleep_for(kStatusPollInterval);
        const auto status = LogAudioStatus(amr, "After play");
        if (status.sound_name == audio_name || status.sound_name == audio_name + ".wav") {
            spdlog::info("AMR reports requested audio '{}' as current sound.", audio_name);
            return;
        }
    }

    spdlog::warn("AMR did not report '{}' as current sound after the play command. If upload "
                 "verification succeeded, check audio format/codec support and robot speaker "
                 "configuration.",
                 audio_name);
}

}  // namespace

int main(int argc, char* argv[]) {
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");

    ProgramOptions program_options;
    try {
        program_options = ParseArguments(argc, argv);
    } catch (const std::exception& e) {
        spdlog::error("Invalid arguments: {}", e.what());
        PrintUsage(argv[0]);
        return 1;
    }

    try {
        flexiv::amr::seer::SeerAmrOptions options;
        options.host = program_options.amr_ip;
        options.auto_connect = false;

        flexiv::amr::seer::SeerAmrClient amr(options);

        spdlog::info("Connecting to SEER AMR: {} ...", options.host);
        amr.Connect();
        spdlog::info("Connected: {}", amr.IsConnected());
        LogAudioStatus(amr, "Initial");

        if (program_options.pause) {
            spdlog::info("Pausing audio playback ...");
            if (!amr.PauseAudio()) {
                throw std::runtime_error("SEER AMR rejected the pause audio command");
            }
            spdlog::info("Audio pause command accepted.");
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            LogAudioStatus(amr, "After pause");
        }

        if (!program_options.download_file.empty()) {
            spdlog::info("Downloading audio '{}' to '{}' ...",
                         program_options.audio_name,
                         program_options.download_file);

            const auto audio_data = amr.DownloadAudio(program_options.audio_name);
            WriteBinaryFile(program_options.download_file, audio_data);

            spdlog::info("Downloaded {} bytes.", audio_data.size());
        }

        if (!program_options.upload_file.empty()) {
            spdlog::info("Uploading audio '{}' from '{}' ...",
                         program_options.audio_name,
                         program_options.upload_file);

            if (!amr.UploadAudio(program_options.upload_file)) {
                throw std::runtime_error("SEER AMR rejected the upload audio command");
            }
            spdlog::info("Audio upload accepted.");
            VerifyUploadedAudio(amr, program_options.audio_name, program_options.upload_file);
        }

        if (!program_options.audio_name.empty() && program_options.download_file.empty()) {
            spdlog::info("Playing audio '{}', loop={} ...",
                         program_options.audio_name,
                         program_options.loop);
            if (!amr.PlayAudio(program_options.audio_name, program_options.loop)) {
                throw std::runtime_error("SEER AMR rejected the play audio command");
            }

            spdlog::info("Audio play command accepted.");
            PollAudioStatusAfterPlay(amr, program_options.audio_name);
        }

        amr.Disconnect();
        spdlog::info("Audio example completed.");
        return 0;
    } catch (const flexiv::amr::AmrException& e) {
        spdlog::error("AMR exception: {}, code: {}", e.what(), static_cast<int>(e.code()));
    } catch (const std::exception& e) {
        spdlog::error("std exception: {}", e.what());
    }

    return 1;
}
