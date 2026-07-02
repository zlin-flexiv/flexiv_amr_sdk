#!/usr/bin/env python3

import argparse
import logging
import signal
import time
from pathlib import Path

import flexiv_amr


logger = logging.getLogger("seer_audio_example")
stop_requested = False
UPLOAD_READY_DELAY_SECONDS = 1.0
STATUS_POLL_INTERVAL_SECONDS = 0.3
STATUS_POLL_COUNT = 8


def signal_handler(signum, frame) -> None:
    global stop_requested
    stop_requested = True
    logger.info("Stop signal received.")


def is_audio_file_path(path: str) -> bool:
    return Path(path).suffix.lower() == ".wav"


def audio_name_from_path(path: str) -> str:
    name = Path(path).stem
    if not name:
        raise ValueError("audio file path must contain a file name")
    return name


def parse_arguments() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Upload, download, play, or pause audio on a SEER AMR."
    )
    parser.add_argument("amr_ip", help="SEER AMR IPv4 address or hostname")
    parser.add_argument(
        "audio",
        nargs="?",
        help="Audio name stored on the AMR, or a local .wav file to upload first",
    )
    parser.add_argument(
        "--loop",
        action="store_true",
        help="Loop playback when playing audio",
    )
    parser.add_argument(
        "--download",
        metavar="OUTPUT_FILE",
        help="Download the named audio to this local file path",
    )
    parser.add_argument(
        "--pause",
        action="store_true",
        help="Pause current audio playback",
    )
    args = parser.parse_args()

    if not args.pause and not args.audio:
        parser.error("audio is required unless --pause is used alone")
    if args.download and not args.audio:
        parser.error("audio is required when downloading audio")
    if args.audio and Path(args.audio).suffix.lower() == ".mp3":
        parser.error("only .wav audio files are supported")

    return args


def log_audio_status(amr: flexiv_amr.SeerAmrClient, context: str):
    status = amr.get_audio_status()
    logger.info(
        "%s audio status: status=%s, sound_name='%s', loop=%s, count=%s",
        context,
        status.status,
        status.sound_name,
        status.loop,
        status.count,
    )
    return status


def verify_uploaded_audio(
    amr: flexiv_amr.SeerAmrClient, audio_name: str, upload_file: str
) -> None:
    logger.info(
        "Waiting %.0f ms for the AMR to index uploaded audio ...",
        UPLOAD_READY_DELAY_SECONDS * 1000.0,
    )
    time.sleep(UPLOAD_READY_DELAY_SECONDS)

    local_data = Path(upload_file).read_bytes()
    logger.info("Verifying upload by downloading audio '%s' ...", audio_name)
    downloaded_data = amr.download_audio(audio_name)

    if len(downloaded_data) != len(local_data):
        logger.warning(
            "Downloaded size %d differs from local size %d. The upload was readable "
            "by name, but the bytes are not identical.",
            len(downloaded_data),
            len(local_data),
        )
        return

    if downloaded_data != local_data:
        logger.warning(
            "Downloaded audio '%s' has the same size as the local file but different content.",
            audio_name,
        )
        return

    logger.info(
        "Upload verification succeeded: '%s' is readable by name and bytes match.",
        audio_name,
    )


def poll_audio_status_after_play(amr: flexiv_amr.SeerAmrClient, audio_name: str) -> None:
    accepted_names = {audio_name, f"{audio_name}.wav"}
    for _ in range(STATUS_POLL_COUNT):
        time.sleep(STATUS_POLL_INTERVAL_SECONDS)
        status = log_audio_status(amr, "After play")
        if status.sound_name in accepted_names:
            logger.info("AMR reports requested audio '%s' as current sound.", audio_name)
            return

    logger.warning(
        "AMR did not report '%s' as current sound after the play command. If upload "
        "verification succeeded, check audio format/codec support and robot speaker configuration.",
        audio_name,
    )


def main() -> int:
    logging.basicConfig(
        level=logging.INFO,
        format="[%(asctime)s.%(msecs)03d] [%(levelname)s] %(message)s",
        datefmt="%Y-%m-%d %H:%M:%S",
    )
    args = parse_arguments()

    signal.signal(signal.SIGINT, signal_handler)
    signal.signal(signal.SIGTERM, signal_handler)

    options = flexiv_amr.SeerAmrOptions()
    options.host = args.amr_ip
    options.auto_connect = False
    options.auto_reconnect = True

    amr = flexiv_amr.SeerAmrClient(options)
    connected = False

    try:
        logger.info("Connecting to SEER AMR: %s ...", args.amr_ip)
        amr.connect()
        connected = True
        logger.info("Connected: %s", amr.is_connected())

        log_audio_status(amr, "Initial")

        if args.pause:
            logger.info("Pausing audio playback ...")
            if not amr.pause_audio():
                raise RuntimeError("SEER AMR rejected the pause audio command")
            logger.info("Audio pause command accepted.")
            time.sleep(0.2)
            log_audio_status(amr, "After pause")

        audio_name = args.audio or ""
        upload_file = ""
        if audio_name and is_audio_file_path(audio_name):
            upload_file = audio_name
            audio_name = audio_name_from_path(upload_file)

        if args.download:
            logger.info("Downloading audio '%s' to '%s' ...", audio_name, args.download)
            audio_data = amr.download_audio(audio_name)
            Path(args.download).write_bytes(audio_data)
            logger.info("Downloaded %d bytes.", len(audio_data))

        if upload_file:
            logger.info("Uploading audio '%s' from '%s' ...", audio_name, upload_file)
            if not amr.upload_audio(upload_file):
                raise RuntimeError("SEER AMR rejected the upload audio command")
            logger.info("Audio upload accepted.")
            verify_uploaded_audio(amr, audio_name, upload_file)

        if audio_name and not args.download and not stop_requested:
            logger.info("Playing audio '%s', loop=%s ...", audio_name, args.loop)
            if not amr.play_audio(audio_name, args.loop):
                raise RuntimeError("SEER AMR rejected the play audio command")
            logger.info("Audio play command accepted.")
            poll_audio_status_after_play(amr, audio_name)

        logger.info("Disconnecting...")
        amr.disconnect()
        connected = False
        logger.info("Audio example completed.")
        return 0

    except flexiv_amr.AmrException as e:
        logger.error("AMR exception: %s", e)

    except Exception as e:
        logger.exception("Unexpected exception: %s", e)

    finally:
        if connected:
            try:
                amr.disconnect()
            except Exception:
                pass

    return 1


if __name__ == "__main__":
    raise SystemExit(main())
