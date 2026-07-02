#!/usr/bin/env python3

import argparse
import logging
import signal
import time

import flexiv_amr


logger = logging.getLogger("seer_basic_example")
stop_requested = False


def setup_logging() -> None:
    logging.basicConfig(
        level=logging.INFO,
        format="[%(asctime)s.%(msecs)03d] [%(levelname)s] %(message)s",
        datefmt="%Y-%m-%d %H:%M:%S",
    )


def signal_handler(signum, frame) -> None:
    global stop_requested
    stop_requested = True
    logger.info("Stop signal received.")


def create_seer_options(amr_ip: str) -> flexiv_amr.SeerAmrOptions:
    options = flexiv_amr.SeerAmrOptions()

    options.host = amr_ip
    options.auto_connect = False
    options.auto_reconnect = True

    # Optional: adjust timeout settings if needed.
    # options.time_out.connect_timeout_ms = 3000
    # options.time_out.send_timeout_ms = 3000
    # options.time_out.recv_timeout_ms = 3000

    return options


def print_amr_info(amr: flexiv_amr.SeerAmrClient) -> None:
    info = amr.get_amr_info()

    logger.info("=== AMR Info ===")
    logger.info("AMR ID: %s", info.amr_id)
    logger.info("AMR Name: %s", info.amr_name)
    logger.info("Model: %s", info.amr_model_name)
    logger.info("Version: %s", info.robokit_version)
    logger.info("Current Map: %s", info.current_map)


def print_runtime_status(amr: flexiv_amr.SeerAmrClient, loop_count: int) -> None:
    logger.info("========== Loop %d ==========", loop_count)

    pose = amr.get_amr_pose()
    logger.info(
        "Pose: x=%.3f, y=%.3f, angle=%.3f, confidence=%.3f, "
        "localization_method=%s, current_station=%s, last_station=%s",
        pose.x,
        pose.y,
        pose.angle,
        pose.confidence,
        pose.localization_method,
        pose.current_station,
        pose.last_station,
    )

    battery = amr.get_battery_status()
    logger.info(
        "Battery: level=%.1f, voltage=%.3f, current=%.3f, charging=%s",
        battery.battery_level,
        battery.voltage,
        battery.current,
        battery.charging,
    )

    navigation = amr.get_navigation_status()
    logger.info(
        "Navigation: task_status=%s, task_type=%s, target_id=%s",
        navigation.task_status,
        navigation.task_type,
        navigation.target_id,
    )

    task = amr.get_task_status()
    logger.info(
        "Task: percentage=%.1f, distance=%.3f, closest_target=%s, "
        "source_name=%s, target_name=%s, task_states.size=%d",
        task.percentage,
        task.distance,
        task.closest_target,
        task.source_name,
        task.target_name,
        len(task.task_states),
    )

    location_status = amr.get_location_status()
    logger.info("Location: reloc_status=%s", location_status)

    ultrasonic = amr.get_ultrasonic_data()
    logger.info("Ultrasonic: %d nodes", len(ultrasonic.ultrasonic_nodes))
    for node in ultrasonic.ultrasonic_nodes:
        logger.info(
            "Ultrasonic node: id=%s, dist=%.3f m, valid=%s",
            node.id,
            node.dist,
            node.valid,
        )

    encoder_data = amr.get_encoder_data()
    for motor_encoder in encoder_data.motor_encoders:
        logger.info("Motor encoder: %s", motor_encoder)
    if encoder_data.legacy_encoder_pulses:
        logger.info("Legacy encoder pulses: %s", encoder_data.legacy_encoder_pulses)

    imu = amr.get_imu_data()
    logger.info(
        "IMU: yaw=%.6f, roll=%.6f, pitch=%.6f, "
        "acc=[%.3f, %.3f, %.3f], rot=[%.3f, %.3f, %.3f]",
        imu.yaw,
        imu.roll,
        imu.pitch,
        imu.acc_x,
        imu.acc_y,
        imu.acc_z,
        imu.rot_x,
        imu.rot_y,
        imu.rot_z,
    )

    logger.info("IsConnected: %s", amr.is_connected())


def main() -> int:
    setup_logging()

    parser = argparse.ArgumentParser(description="Basic Python example for SEER AMR.")
    parser.add_argument(
        "amr_ip",
        help="SEER AMR IP address, for example: 192.168.192.5",
    )
    parser.add_argument(
        "interval_ms",
        nargs="?",
        type=int,
        default=1000,
        help="Polling interval in milliseconds. Default: 1000",
    )

    args = parser.parse_args()

    signal.signal(signal.SIGINT, signal_handler)
    signal.signal(signal.SIGTERM, signal_handler)

    options = create_seer_options(args.amr_ip)

    logger.info("connect_timeout_ms: %d", options.time_out.connect_timeout_ms)
    logger.info("send_timeout_ms: %d", options.time_out.send_timeout_ms)
    logger.info("recv_timeout_ms: %d", options.time_out.recv_timeout_ms)

    amr = flexiv_amr.SeerAmrClient(options)

    try:
        logger.info("Connecting to SEER AMR: %s ...", args.amr_ip)
        amr.connect()

        logger.info("Connected: %s", amr.is_connected())

        print_amr_info(amr)

        loop_count = 0

        while not stop_requested:
            loop_count += 1

            try:
                print_runtime_status(amr, loop_count)

            except flexiv_amr.AmrException as e:
                logger.error("AMR exception in loop %d: %s", loop_count, e)
                logger.warning("Breaking loop because AMR communication failed.")
                break

            except Exception as e:
                logger.exception("Unexpected exception in loop %d: %s", loop_count, e)
                logger.warning("Breaking loop because unexpected exception occurred.")
                break

            time.sleep(args.interval_ms / 1000.0)

        logger.info("Disconnecting...")
        amr.disconnect()
        logger.info("Disconnected.")

        return 0

    except flexiv_amr.AmrException as e:
        logger.error("AMR exception: %s", e)

    except Exception as e:
        logger.exception("Unexpected exception: %s", e)

    finally:
        try:
            if amr.is_connected():
                amr.disconnect()
        except Exception:
            pass

    return 1


if __name__ == "__main__":
    raise SystemExit(main())
