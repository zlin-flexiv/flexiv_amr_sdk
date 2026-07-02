#!/usr/bin/env python3

import argparse
import logging
import math
import signal
import time
from typing import List, Optional

import flexiv_amr


logger = logging.getLogger("seer_navigation_example")
stop_requested = False

NAVIGATION_RUNNING = 2
NAVIGATION_COMPLETED = 4
NAVIGATION_FAILED = 5
NAVIGATION_CANCELED = 6
STATUS_REQUEST_GAP_SECONDS = 0.15
POLL_INTERVAL_SECONDS = 0.35
PAUSE_DURATION_SECONDS = 2.0


def signal_handler(signum, frame) -> None:
    global stop_requested
    stop_requested = True
    logger.info("Stop signal received.")


def finite_float(value: str) -> float:
    parsed = float(value)
    if not math.isfinite(parsed):
        raise argparse.ArgumentTypeError("value must be finite")
    return parsed


def parse_arguments() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description=(
            "Execute fixed or specified path navigation and monitor status "
            "until completion."
        ),
        epilog=(
            "One target uses API 3051. Multiple targets use API 3066 and must "
            "describe directly connected map lines. This example is for "
            "single-AMR validation, not fleet dispatch."
        ),
    )
    parser.add_argument("amr_ip", help="SEER AMR IPv4 address or hostname")
    parser.add_argument("source_id", help="Source station ID")
    parser.add_argument(
        "target_ids",
        nargs="+",
        help="One or more target station IDs in navigation order",
    )

    parser.add_argument(
        "--pause-after",
        type=finite_float,
        metavar="SECONDS",
        help="Pause once, wait two seconds, then resume",
    )
    parser.add_argument(
        "--timeout",
        type=finite_float,
        default=300.0,
        metavar="SECONDS",
        help="Navigation timeout (default: 300)",
    )

    args = parser.parse_args()
    if args.pause_after is not None and args.pause_after < 0.0:
        parser.error("--pause-after must be non-negative")
    if args.timeout <= 0.0:
        parser.error("--timeout must be greater than zero")
    return args


def sleep_interruptibly(duration_seconds: float) -> None:
    deadline = time.monotonic() + duration_seconds
    while not stop_requested:
        remaining = deadline - time.monotonic()
        if remaining <= 0.0:
            return
        time.sleep(min(remaining, 0.05))


def make_task_id(index: int) -> str:
    return f"seer_navigation_example_{time.time_ns()}_{index}"


def build_navigation_commands(
    source_id: str, target_ids: List[str]
) -> List[flexiv_amr.SeerPathNaviCommand]:
    commands = []
    current_source = source_id

    for index, target_id in enumerate(target_ids):
        command = flexiv_amr.SeerPathNaviCommand()
        command.source_id = current_source
        command.id = target_id
        command.task_id = make_task_id(index)
        commands.append(command)
        current_source = target_id

    return commands


def wait_for_navigation(
    amr: flexiv_amr.SeerAmrClient,
    pause_after_seconds: Optional[float],
    timeout_seconds: float,
) -> str:
    start = time.monotonic()
    deadline = start + timeout_seconds
    pause_exercised = False

    while not stop_requested and time.monotonic() < deadline:
        navigation = amr.get_navigation_status()
        sleep_interruptibly(STATUS_REQUEST_GAP_SECONDS)
        if stop_requested:
            return "interrupted"

        task = amr.get_task_status()
        logger.info(
            "Navigation: status=%s, type=%s, target=%s, finished=%d, unfinished=%d",
            navigation.task_status,
            navigation.task_type,
            navigation.target_id,
            len(navigation.finished_path),
            len(navigation.unfinished_path),
        )
        logger.info(
            "Task: percentage=%.1f, source=%s, target=%s, states=%d, info=%s",
            task.percentage,
            task.source_name,
            task.target_name,
            len(task.task_states),
            task.info,
        )

        if navigation.task_status == NAVIGATION_COMPLETED:
            return "completed"
        if navigation.task_status == NAVIGATION_FAILED:
            return "failed"
        if navigation.task_status == NAVIGATION_CANCELED:
            return "canceled"

        elapsed_seconds = time.monotonic() - start
        if (
            not pause_exercised
            and pause_after_seconds is not None
            and navigation.task_status == NAVIGATION_RUNNING
            and elapsed_seconds >= pause_after_seconds
        ):
            logger.warning("Pausing navigation for two seconds...")
            if not amr.pause_navigation():
                raise RuntimeError("SEER AMR rejected the pause command")
            sleep_interruptibly(PAUSE_DURATION_SECONDS)
            if stop_requested:
                return "interrupted"
            if not amr.resume_navigation():
                raise RuntimeError("SEER AMR rejected the resume command")
            logger.info("Navigation resumed.")
            pause_exercised = True

        sleep_interruptibly(POLL_INTERVAL_SECONDS)

    return "interrupted" if stop_requested else "timed_out"


def main() -> int:
    logging.basicConfig(
        level=logging.INFO,
        format="[%(asctime)s.%(msecs)03d] [%(levelname)s] %(message)s",
        datefmt="%Y-%m-%d %H:%M:%S",
    )
    args = parse_arguments()

    signal.signal(signal.SIGINT, signal_handler)
    signal.signal(signal.SIGTERM, signal_handler)

    client_options = flexiv_amr.SeerAmrOptions()
    client_options.host = args.amr_ip
    client_options.auto_connect = False
    client_options.auto_reconnect = True

    amr = flexiv_amr.SeerAmrClient(client_options)
    navigation_active = False
    connected = False
    exit_code = 1

    try:
        logger.info("Connecting to SEER AMR: %s ...", args.amr_ip)
        amr.connect()
        connected = True
        logger.info("Connected: %s", amr.is_connected())

        emergency = amr.get_emergency_status()
        if emergency.emergency or emergency.driver_emc or emergency.soft_emc:
            raise RuntimeError(
                "AMR emergency stop is active; navigation is not started"
            )

        sleep_interruptibly(STATUS_REQUEST_GAP_SECONDS)
        block = amr.get_block_status()
        if block.blocked:
            logger.warning(
                "AMR is currently blocked, reason=%s, sensor_id=%s",
                block.block_reason,
                block.block_id,
            )

        if stop_requested:
            raise RuntimeError("navigation interrupted before start")

        commands = build_navigation_commands(args.source_id, args.target_ids)
        if len(commands) == 1:
            command = commands[0]
            logger.warning(
                "Starting fixed path navigation: %s -> %s (task_id=%s)",
                command.source_id,
                command.id,
                command.task_id,
            )
            if not amr.execute_fixed_path_navigation(command):
                raise RuntimeError("SEER AMR rejected fixed path navigation")
        else:
            logger.warning(
                "Starting specified path navigation with %d directly connected segments",
                len(commands),
            )
            for command in commands:
                logger.info(
                    "Segment: %s -> %s (task_id=%s)",
                    command.source_id,
                    command.id,
                    command.task_id,
                )
            if not amr.execute_specified_path_navigation(commands):
                raise RuntimeError("SEER AMR rejected specified path navigation")
        navigation_active = True

        sleep_interruptibly(STATUS_REQUEST_GAP_SECONDS)
        result = wait_for_navigation(amr, args.pause_after, args.timeout)
        if result == "completed":
            navigation_active = False
            exit_code = 0
            logger.info("Navigation completed successfully.")
        elif result == "failed":
            navigation_active = False
            logger.error("Navigation failed.")
        elif result == "canceled":
            navigation_active = False
            logger.warning("Navigation was canceled by the controller.")
        elif result == "timed_out":
            logger.error("Navigation timed out.")
        else:
            logger.warning("Navigation interrupted by stop signal.")
    except flexiv_amr.AmrException as exc:
        logger.error("AMR exception: %s", exc)
    except Exception as exc:
        logger.exception("Unexpected exception: %s", exc)
    finally:
        if navigation_active:
            try:
                logger.warning("Canceling active navigation during cleanup...")
                amr.cancel_navigation()
            except Exception as exc:
                logger.exception("Failed to cancel navigation during cleanup: %s", exc)

        if connected:
            try:
                amr.disconnect()
            except Exception as exc:
                logger.exception("Failed to disconnect during cleanup: %s", exc)

    return exit_code


if __name__ == "__main__":
    raise SystemExit(main())
