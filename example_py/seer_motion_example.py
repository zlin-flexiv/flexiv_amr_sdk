#!/usr/bin/env python3

import argparse
import logging
import signal
import time

import flexiv_amr


logger = logging.getLogger("seer_motion_example")
stop_requested = False

LINEAR_VELOCITY_X = 0.1
LINEAR_VELOCITY_Y = 0.0
ANGULAR_VELOCITY = 0.1
MOTION_DURATION_MS = 3000.0
WAIT_INTERVAL_SECONDS = 0.05


def signal_handler(signum, frame) -> None:
    global stop_requested
    stop_requested = True
    logger.info("Stop signal received.")


def main() -> int:
    logging.basicConfig(
        level=logging.INFO,
        format="[%(asctime)s.%(msecs)03d] [%(levelname)s] %(message)s",
        datefmt="%Y-%m-%d %H:%M:%S",
    )

    parser = argparse.ArgumentParser(
        description="Drive a SEER AMR with one three-second open-loop velocity command."
    )
    parser.add_argument(
        "amr_ip", help="SEER AMR IP address, for example: 192.168.192.5"
    )
    args = parser.parse_args()

    signal.signal(signal.SIGINT, signal_handler)
    signal.signal(signal.SIGTERM, signal_handler)

    options = flexiv_amr.SeerAmrOptions()
    options.host = args.amr_ip
    options.auto_connect = False
    options.auto_reconnect = True

    amr = flexiv_amr.SeerAmrClient(options)
    motion_started = False
    exit_code = 0

    try:
        logger.info("Connecting to SEER AMR: %s ...", args.amr_ip)
        amr.connect()
        logger.info("Connected: %s", amr.is_connected())

        command = flexiv_amr.SeerOpenLoopMotionCommand()
        command.vx = LINEAR_VELOCITY_X
        command.vy = LINEAR_VELOCITY_Y
        command.w = ANGULAR_VELOCITY
        command.duration = MOTION_DURATION_MS

        logger.warning(
            "Starting open-loop motion: vx=%.3f m/s, vy=%.3f m/s, "
            "w=%.3f rad/s, duration=%.0f ms",
            LINEAR_VELOCITY_X,
            LINEAR_VELOCITY_Y,
            ANGULAR_VELOCITY,
            MOTION_DURATION_MS,
        )

        if not amr.execute_open_loop_motion(command):
            raise RuntimeError("SEER AMR rejected the open-loop motion command")
        motion_started = True

        deadline = time.monotonic() + MOTION_DURATION_MS / 1000.0
        while not stop_requested and time.monotonic() < deadline:
            time.sleep(WAIT_INTERVAL_SECONDS)

        logger.info("Motion duration elapsed or stop requested.")

    except flexiv_amr.AmrException as e:
        logger.error("AMR exception: %s", e)
        exit_code = 1
    except Exception as e:
        logger.exception("Unexpected exception: %s", e)
        exit_code = 1
    finally:
        if motion_started:
            try:
                logger.info("Stopping open-loop motion...")
                if not amr.stop_open_loop_motion():
                    logger.error("SEER AMR rejected the stop command")
                    exit_code = 1
            except Exception as e:
                logger.exception(
                    "Failed to stop open-loop motion during cleanup: %s", e
                )
                exit_code = 1

        try:
            if amr.is_connected():
                amr.disconnect()
        except Exception as e:
            logger.exception("Failed to disconnect during cleanup: %s", e)
            exit_code = 1

    if exit_code == 0:
        logger.info("Motion example completed.")
    return exit_code


if __name__ == "__main__":
    raise SystemExit(main())
