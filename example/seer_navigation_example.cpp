/**
 * @example seer_navigation_example.cpp
 * This example executes fixed or specified path navigation and monitors navigation and task status
 * until completion.
 */

#include <chrono>
#include <cmath>
#include <csignal>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include <flexiv/amr/exceptions.h>
#include <flexiv/amr/vendor/seer/seer_amr.h>
#include <spdlog/spdlog.h>

namespace {

using flexiv::amr::seer::SeerAmrClient;
using flexiv::amr::seer::SeerPathNaviCommand;

volatile std::sig_atomic_t g_stop_requested = 0;

constexpr int kNavigationRunning = 2;
constexpr int kNavigationCompleted = 4;
constexpr int kNavigationFailed = 5;
constexpr int kNavigationCanceled = 6;
constexpr auto kStatusRequestGap = std::chrono::milliseconds(150);
constexpr auto kPollInterval = std::chrono::milliseconds(350);
constexpr auto kPauseDuration = std::chrono::seconds(2);

struct ExampleOptions {
    std::string amr_ip;
    std::string source_id;
    std::vector<std::string> target_ids;
    std::optional<double> pause_after_seconds;
    double timeout_seconds = 300.0;
};

enum class NavigationResult {
    kCompleted,
    kFailed,
    kCanceled,
    kInterrupted,
    kTimedOut,
};

void SignalHandler(int) {
    g_stop_requested = 1;
}

void PrintUsage(const char* program) {
    std::cerr
        << "Usage:\n"
        << "  " << program << " <amr_ip> <source_id> <target_id> [target_id ...] [options]\n\n"
        << "Options:\n"
        << "  --pause-after <seconds>      Pause once, wait two seconds, then resume.\n"
        << "  --timeout <seconds>          Navigation timeout (default: 300).\n\n"
        << "Examples:\n"
        << "  " << program << " 192.168.192.5 LM1 LM2\n"
        << "  " << program << " 192.168.192.5 LM1 LM2 AP1\n"
        << "  " << program << " 192.168.192.5 LM1 LM2 --pause-after 3\n\n"
        << "One target uses API 3051. Multiple targets use API 3066 and must describe directly "
           "connected map lines. This example is for single-AMR validation, not fleet dispatch.\n";
}

double ParseFiniteDouble(const std::string& text, const char* name) {
    std::size_t parsed = 0;
    const double value = std::stod(text, &parsed);
    if (parsed != text.size() || !std::isfinite(value)) {
        throw std::invalid_argument(std::string(name) + " must be a finite number");
    }
    return value;
}

ExampleOptions ParseOptions(int argc, char* argv[]) {
    if (argc < 4) {
        throw std::invalid_argument("amr_ip, source_id, and at least one target_id are required");
    }

    ExampleOptions options;
    options.amr_ip = argv[1];
    std::vector<std::string> stations;

    for (int i = 2; i < argc; ++i) {
        const std::string argument = argv[i];

        if (argument == "--pause-after") {
            if (i + 1 >= argc) {
                throw std::invalid_argument("--pause-after requires seconds");
            }
            const double seconds = ParseFiniteDouble(argv[++i], "pause-after seconds");
            if (seconds < 0.0) {
                throw std::invalid_argument("pause-after seconds must be non-negative");
            }
            options.pause_after_seconds = seconds;
        } else if (argument == "--timeout") {
            if (i + 1 >= argc) {
                throw std::invalid_argument("--timeout requires seconds");
            }
            options.timeout_seconds = ParseFiniteDouble(argv[++i], "timeout seconds");
            if (options.timeout_seconds <= 0.0) {
                throw std::invalid_argument("timeout seconds must be greater than zero");
            }
        } else if (argument.rfind("--", 0) == 0) {
            throw std::invalid_argument("unknown option: " + argument);
        } else {
            stations.emplace_back(argument);
        }
    }

    if (stations.size() < 2) {
        throw std::invalid_argument("source_id and at least one target_id are required");
    }

    options.source_id = stations.front();
    options.target_ids.assign(stations.begin() + 1, stations.end());
    return options;
}

void SleepInterruptibly(std::chrono::milliseconds duration) {
    constexpr auto kStep = std::chrono::milliseconds(50);
    auto remaining = duration;
    while (!g_stop_requested && remaining > std::chrono::milliseconds::zero()) {
        const auto sleep_duration = remaining < kStep ? remaining : kStep;
        std::this_thread::sleep_for(sleep_duration);
        remaining -= sleep_duration;
    }
}

std::string MakeTaskId(std::size_t index) {
    const auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                               std::chrono::system_clock::now().time_since_epoch())
                               .count();
    return "seer_navigation_example_" + std::to_string(timestamp) + "_" + std::to_string(index);
}

std::vector<SeerPathNaviCommand> BuildNavigationCommands(const ExampleOptions& options) {
    std::vector<SeerPathNaviCommand> commands;
    std::string source_id = options.source_id;

    for (std::size_t index = 0; index < options.target_ids.size(); ++index) {
        SeerPathNaviCommand command;
        command.source_id = source_id;
        command.id = options.target_ids[index];
        command.task_id = MakeTaskId(index);
        commands.emplace_back(std::move(command));
        source_id = options.target_ids[index];
    }

    return commands;
}

NavigationResult WaitForNavigation(SeerAmrClient& amr, const ExampleOptions& options) {
    const auto start = std::chrono::steady_clock::now();
    const auto deadline = start + std::chrono::duration<double>(options.timeout_seconds);
    bool pause_exercised = false;

    while (!g_stop_requested && std::chrono::steady_clock::now() < deadline) {
        const auto navigation = amr.GetNavigationStatus();
        SleepInterruptibly(kStatusRequestGap);
        if (g_stop_requested) {
            return NavigationResult::kInterrupted;
        }

        const auto task = amr.GetTaskStatus();
        spdlog::info("Navigation: status={}, type={}, target={}, finished={}, unfinished={}",
                     navigation.task_status,
                     navigation.task_type,
                     navigation.target_id,
                     navigation.finished_path.size(),
                     navigation.unfinished_path.size());
        spdlog::info("Task: percentage={:.1f}, source={}, target={}, states={}, info={}",
                     task.percentage,
                     task.source_name,
                     task.target_name,
                     task.task_states.size(),
                     task.info);

        if (navigation.task_status == kNavigationCompleted) {
            return NavigationResult::kCompleted;
        }
        if (navigation.task_status == kNavigationFailed) {
            return NavigationResult::kFailed;
        }
        if (navigation.task_status == kNavigationCanceled) {
            return NavigationResult::kCanceled;
        }

        const double elapsed_seconds =
            std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
        if (!pause_exercised && options.pause_after_seconds.has_value() &&
            navigation.task_status == kNavigationRunning &&
            elapsed_seconds >= options.pause_after_seconds.value()) {
            spdlog::warn("Pausing navigation for two seconds...");
            if (!amr.PauseNavigation()) {
                throw std::runtime_error("SEER AMR rejected the pause command");
            }

            SleepInterruptibly(
                std::chrono::duration_cast<std::chrono::milliseconds>(kPauseDuration));
            if (g_stop_requested) {
                return NavigationResult::kInterrupted;
            }

            if (!amr.ResumeNavigation()) {
                throw std::runtime_error("SEER AMR rejected the resume command");
            }
            spdlog::info("Navigation resumed.");
            pause_exercised = true;
        }

        SleepInterruptibly(kPollInterval);
    }

    return g_stop_requested ? NavigationResult::kInterrupted : NavigationResult::kTimedOut;
}

}  // namespace

int main(int argc, char* argv[]) {
    if (argc >= 2 && (std::string(argv[1]) == "--help" || std::string(argv[1]) == "-h")) {
        PrintUsage(argv[0]);
        return 0;
    }

    ExampleOptions example_options;
    try {
        example_options = ParseOptions(argc, argv);
    } catch (const std::exception& e) {
        std::cerr << "Invalid arguments: " << e.what() << "\n\n";
        PrintUsage(argv[0]);
        return 1;
    }

    std::signal(SIGINT, SignalHandler);
    std::signal(SIGTERM, SignalHandler);
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");

    flexiv::amr::seer::SeerAmrOptions client_options;
    client_options.host = example_options.amr_ip;
    client_options.auto_connect = false;
    client_options.auto_reconnect = true;

    SeerAmrClient amr(client_options);
    bool connected = false;
    bool navigation_active = false;
    int exit_code = 1;

    try {
        spdlog::info("Connecting to SEER AMR: {} ...", example_options.amr_ip);
        amr.Connect();
        connected = true;
        spdlog::info("Connected: {}", amr.IsConnected());

        const auto emergency = amr.GetEmergencyStatus();
        if (emergency.emergency || emergency.driver_emc || emergency.soft_emc) {
            throw std::runtime_error("AMR emergency stop is active; navigation is not started");
        }

        SleepInterruptibly(kStatusRequestGap);
        const auto block = amr.GetBlockStatus();
        if (block.blocked) {
            spdlog::warn("AMR is currently blocked, reason={}, sensor_id={}",
                         block.block_reason,
                         block.block_id);
        }

        if (g_stop_requested) {
            throw std::runtime_error("navigation interrupted before start");
        }

        const auto commands = BuildNavigationCommands(example_options);
        if (commands.size() == 1) {
            spdlog::warn("Starting fixed path navigation: {} -> {} (task_id={})",
                         commands.front().source_id.value(),
                         commands.front().id.value(),
                         commands.front().task_id.value());
            if (!amr.ExecuteFixedPathNavigation(commands.front())) {
                throw std::runtime_error("SEER AMR rejected fixed path navigation");
            }
        } else {
            spdlog::warn("Starting specified path navigation with {} directly connected segments",
                         commands.size());
            for (const auto& command : commands) {
                spdlog::info("Segment: {} -> {} (task_id={})",
                             command.source_id.value(),
                             command.id.value(),
                             command.task_id.value());
            }
            if (!amr.ExecuteSpecifiedPathNavigation(commands)) {
                throw std::runtime_error("SEER AMR rejected specified path navigation");
            }
        }
        navigation_active = true;

        SleepInterruptibly(kStatusRequestGap);
        const NavigationResult result = WaitForNavigation(amr, example_options);
        if (result == NavigationResult::kCompleted) {
            navigation_active = false;
            exit_code = 0;
            spdlog::info("Navigation completed successfully.");
        } else if (result == NavigationResult::kFailed) {
            navigation_active = false;
            spdlog::error("Navigation failed.");
        } else if (result == NavigationResult::kCanceled) {
            navigation_active = false;
            spdlog::warn("Navigation was canceled by the controller.");
        } else if (result == NavigationResult::kTimedOut) {
            spdlog::error("Navigation timed out.");
        } else {
            spdlog::warn("Navigation interrupted by stop signal.");
        }
    } catch (const flexiv::amr::AmrException& e) {
        spdlog::error("AMR exception: {}, code: {}", e.what(), static_cast<int>(e.code()));
    } catch (const std::exception& e) {
        spdlog::error("std exception: {}", e.what());
    }

    if (navigation_active) {
        try {
            spdlog::warn("Canceling active navigation during cleanup...");
            amr.CancelNavigation();
        } catch (const std::exception& e) {
            spdlog::error("Failed to cancel navigation during cleanup: {}", e.what());
        }
    }

    if (connected) {
        try {
            amr.Disconnect();
        } catch (const std::exception& e) {
            spdlog::error("Failed to disconnect during cleanup: {}", e.what());
        }
    }

    return exit_code;
}
