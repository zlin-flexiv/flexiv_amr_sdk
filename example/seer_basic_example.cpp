/**
 * @example seer_basic_example.cpp
 * This example connects to a SEER AMR and prints basic status information.
 */

#include <atomic>
#include <chrono>
#include <csignal>
#include <iostream>
#include <string>
#include <thread>

#include <flexiv/amr/exceptions.h>
#include <flexiv/amr/vendor/seer/seer_amr.h>
#include <spdlog/spdlog.h>

namespace {
volatile std::sig_atomic_t g_stop_requested = 0;

void SignalHandler(int) {
    g_stop_requested = 1;
}
}  // namespace

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage:\n"
                  << "  " << argv[0] << " <amr_ip> [interval_ms]\n\n"
                  << "Example:\n"
                  << "  " << argv[0] << " 192.168.192.5 1000\n";
        return 1;
    }

    std::signal(SIGINT, SignalHandler);
    std::signal(SIGTERM, SignalHandler);

    const std::string amr_ip = argv[1];
    const int interval_ms = argc >= 3 ? std::stoi(argv[2]) : 1000;

    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");

    try {
        flexiv::amr::seer::SeerAmrOptions options;
        options.host = amr_ip;
        options.auto_connect = false;

        spdlog::info("connect_timeout_ms: {}", options.time_out.connect_timeout_ms);
        spdlog::info("send_timeout_ms: {}", options.time_out.send_timeout_ms);
        spdlog::info("recv_timeout_ms: {}", options.time_out.recv_timeout_ms);

        flexiv::amr::seer::SeerAmrClient amr(options);

        spdlog::info("Connecting to SEER AMR: {} ...", amr_ip);
        amr.Connect();

        spdlog::info("Connected: {}", amr.IsConnected());

        {
            const auto info = amr.GetAmrInfo();

            spdlog::info("=== AMR Info ===");
            spdlog::info("AMR ID: {}", info.amr_id);
            spdlog::info("AMR Name: {}", info.amr_name);
            spdlog::info("Model: {}", info.amr_model_name);
            spdlog::info("Version: {}", info.robokit_version);
            spdlog::info("Current Map: {}", info.current_map);
        }

        std::size_t loop_count = 0;

        while (!g_stop_requested) {
            ++loop_count;

            try {
                spdlog::info("========== Loop {} ==========", loop_count);

                const auto pose = amr.GetAmrPose();
                spdlog::info("Pose: x={:.3f}, y={:.3f}, angle={:.3f}, confidence={:.3f}, "
                             "localization_method={}, current_station={}, last_station={}",
                             pose.x,
                             pose.y,
                             pose.angle,
                             pose.confidence,
                             pose.localization_method,
                             pose.current_station,
                             pose.last_station);

                const auto battery = amr.GetBatteryStatus();
                spdlog::info("Battery: level={:.1f}, voltage={:.3f}, current={:.3f}, charging={}",
                             battery.battery_level,
                             battery.voltage,
                             battery.current,
                             battery.charging);

                const auto navigation = amr.GetNavigationStatus();
                spdlog::info("Navigation: task_status={}, task_type={}, target_id={}",
                             navigation.task_status,
                             navigation.task_type,
                             navigation.target_id);

                const auto task = amr.GetTaskStatus();
                spdlog::info("Task: percentage={:.1f}, distance={:.3f}, closest_target={}, "
                             "source_name={}, target_name={}, task_states.size={}",
                             task.percentage,
                             task.distance,
                             task.closest_target,
                             task.source_name,
                             task.target_name,
                             task.task_states.size());

                const int location_status = amr.GetLocationStatus();
                spdlog::info("Location: reloc_status={}", location_status);

                const auto ultrasonic = amr.GetUltrasonicData();
                spdlog::info("Ultrasonic: {} nodes", ultrasonic.ultrasonic_nodes.size());
                for (const auto& node : ultrasonic.ultrasonic_nodes) {
                    spdlog::info("Ultrasonic node: id={}, dist={:.3f} m, valid={}",
                                 node.id,
                                 node.dist,
                                 node.valid);
                }

                const auto encoder_data = amr.GetEncoderData();
                for (const auto& motor_encoder : encoder_data.motor_encoders) {
                    spdlog::info("Motor encoder: {}", motor_encoder.dump());
                }
                if (!encoder_data.legacy_encoder_pulses.empty()) {
                    spdlog::info("Legacy encoder pulses: {} values",
                                 encoder_data.legacy_encoder_pulses.size());
                }

                const auto imu = amr.GetImuData();
                spdlog::info("IMU: yaw={:.6f}, roll={:.6f}, pitch={:.6f}, "
                             "acc=[{:.3f}, {:.3f}, {:.3f}], rot=[{:.3f}, {:.3f}, {:.3f}]",
                             imu.yaw,
                             imu.roll,
                             imu.pitch,
                             imu.acc_x,
                             imu.acc_y,
                             imu.acc_z,
                             imu.rot_x,
                             imu.rot_y,
                             imu.rot_z);

                spdlog::info("IsConnected: {}", amr.IsConnected());
            } catch (const flexiv::amr::AmrException& e) {
                spdlog::error("AMR exception in loop {}: {}, code: {}",
                              loop_count,
                              e.what(),
                              static_cast<int>(e.code()));

                spdlog::warn("Breaking loop because AMR communication failed.");
                break;
            } catch (const std::exception& e) {
                spdlog::error("std exception in loop {}: {}", loop_count, e.what());
                spdlog::warn("Breaking loop because unexpected exception occurred.");
                break;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(interval_ms));
        }

        spdlog::info("Disconnecting...");
        amr.Disconnect();
        spdlog::info("Disconnected.");

        return 0;
    } catch (const flexiv::amr::AmrException& e) {
        spdlog::error("AMR exception: {}, code: {}", e.what(), static_cast<int>(e.code()));
    } catch (const std::exception& e) {
        spdlog::error("std exception: {}", e.what());
    }

    return 1;
}
