/**
 * @example seer_motion_example.cpp
 * This example drives a SEER AMR with one three-second open-loop velocity command.
 */

#include <chrono>
#include <cstdint>
#include <csignal>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>

#include <flexiv/amr/exceptions.h>
#include <flexiv/amr/vendor/seer/seer_amr.h>
#include <spdlog/spdlog.h>

namespace {
volatile std::sig_atomic_t g_stop_requested = 0;

constexpr double kLinearVelocityX = 0.1;
constexpr double kLinearVelocityY = 0.0;
constexpr double kAngularVelocity = 0.1;
constexpr std::int64_t kMotionDurationMs = 3000;
constexpr auto kWaitInterval = std::chrono::milliseconds(50);

void SignalHandler(int) {
    g_stop_requested = 1;
}
}  // namespace

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage:\n"
                  << "  " << argv[0] << " <amr_ip>\n\n"
                  << "Example:\n"
                  << "  " << argv[0] << " 192.168.192.5\n";
        return 1;
    }

    std::signal(SIGINT, SignalHandler);
    std::signal(SIGTERM, SignalHandler);

    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");

    flexiv::amr::seer::SeerAmrOptions options;
    options.host = argv[1];
    options.auto_connect = false;

    flexiv::amr::seer::SeerAmrClient amr(options);
    bool motion_started = false;

    try {
        spdlog::info("Connecting to SEER AMR: {} ...", options.host);
        amr.Connect();
        spdlog::info("Connected: {}", amr.IsConnected());

        flexiv::amr::seer::SeerOpenLoopMotionCommand command;
        command.vx = kLinearVelocityX;
        command.vy = kLinearVelocityY;
        command.w = kAngularVelocity;
        command.duration = kMotionDurationMs;

        spdlog::warn("Starting open-loop motion: vx={} m/s, vy={} m/s, w={} rad/s, duration={} ms",
                     kLinearVelocityX,
                     kLinearVelocityY,
                     kAngularVelocity,
                     kMotionDurationMs);

        if (!amr.ExecuteOpenLoopMotion(command)) {
            throw std::runtime_error("SEER AMR rejected the open-loop motion command");
        }
        motion_started = true;

        const auto deadline =
            std::chrono::steady_clock::now() + std::chrono::milliseconds(kMotionDurationMs);
        while (!g_stop_requested && std::chrono::steady_clock::now() < deadline) {
            std::this_thread::sleep_for(kWaitInterval);
        }

        spdlog::info("Stopping open-loop motion...");
        if (!amr.StopOpenLoopMotion()) {
            throw std::runtime_error("SEER AMR rejected the stop command");
        }
        motion_started = false;

        amr.Disconnect();
        spdlog::info("Motion example completed.");
        return 0;
    } catch (const flexiv::amr::AmrException& e) {
        spdlog::error("AMR exception: {}, code: {}", e.what(), static_cast<int>(e.code()));
    } catch (const std::exception& e) {
        spdlog::error("std exception: {}", e.what());
    }

    if (motion_started) {
        try {
            spdlog::warn("Attempting to stop open-loop motion during cleanup...");
            amr.StopOpenLoopMotion();
        } catch (const std::exception& e) {
            spdlog::error("Failed to stop open-loop motion during cleanup: {}", e.what());
        }
    }

    try {
        amr.Disconnect();
    } catch (const std::exception& e) {
        spdlog::error("Failed to disconnect during cleanup: {}", e.what());
    }

    return 1;
}
