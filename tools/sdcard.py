#!/usr/bin/env python3
"""Talk to the CataBox SD diagnostic firmware over USB serial.

Examples (use the PlatformIO venv, which has pyserial):
  ~/.venvs/platformio/bin/python tools/sdcard.py list
  ~/.venvs/platformio/bin/python tools/sdcard.py format
  ~/.venvs/platformio/bin/python tools/sdcard.py upload story.mp3 6062BD5C.mp3

Flash the diagnostic first:
  pio run -e diag_sd -t upload
Close any open serial monitor before running this script.
"""

import argparse
import os
import sys
import time

import serial

DEFAULT_PORT = "/dev/cu.usbserial-0001"


def log(text):
    sys.stdout.write(text)
    sys.stdout.flush()


def wait_for(ser, tokens, timeout):
    if isinstance(tokens, str):
        tokens = [tokens]
    deadline = time.time() + timeout
    seen = []
    while time.time() < deadline:
        line = ser.readline()
        if not line:
            continue
        text = line.decode("utf-8", "replace")
        log(text)
        seen.append(text)
        for token in tokens:
            if token in text:
                return token
    raise TimeoutError("timed out waiting for " + " / ".join(tokens))


def send_command(ser, command, tokens, timeout):
    ser.reset_input_buffer()
    ser.write((command + "\n").encode())
    ser.flush()
    return wait_for(ser, tokens, timeout)


def do_list(ser):
    send_command(ser, "l", "LIST OK", 10)


def do_format(ser):
    result = send_command(ser, "f", ["FORMAT OK", "FORMAT FAILED"], 90)
    if result == "FORMAT FAILED":
        raise RuntimeError("format failed; check wiring/power and the serial log")


def do_upload(ser, local, remote):
    size = os.path.getsize(local)
    if size == 0:
        raise RuntimeError("refusing to upload an empty file")
    if size > 64 * 1024 * 1024:
        raise RuntimeError("file too large (diagnostic limit is 64 MB)")

    ser.reset_input_buffer()
    ser.write(("u %s %d\n" % (remote, size)).encode())
    ser.flush()
    wait_for(ser, "READY", 10)

    sent = 0
    with open(local, "rb") as handle:
        while True:
            chunk = handle.read(4096)
            if not chunk:
                break
            ser.write(chunk)
            sent += len(chunk)
            if sent % (64 * 1024) < 4096:
                log("\r  sent %d / %d bytes" % (sent, size))
    ser.flush()
    log("\r  sent %d / %d bytes\n" % (sent, size))

    timeout = max(120, int(size / 8000) + 60)
    result = wait_for(ser, ["OK ", "ERR "], timeout)
    if result == "ERR ":
        raise RuntimeError("upload failed; see the serial log above")
    wait_for(ser, "LIST OK", 10)


def main():
    parser = argparse.ArgumentParser(description="CataBox SD-card diagnostic client")
    parser.add_argument("--port", default=DEFAULT_PORT)
    parser.add_argument("--baud", type=int, default=115200)
    sub = parser.add_subparsers(dest="command", required=True)

    sub.add_parser("list", help="list /stories")
    sub.add_parser("format", help="format the card as FAT if it has no filesystem")

    upload = sub.add_parser("upload", help="upload a file into /stories")
    upload.add_argument("local")
    upload.add_argument("remote", nargs="?", help="name on the card (default: basename)")

    args = parser.parse_args()

    try:
        ser = serial.Serial(args.port, args.baud, timeout=0.5, write_timeout=60)
    except serial.SerialException as exc:
        print("Could not open %s: %s" % (args.port, exc))
        print("Close any serial monitor using the port and try again.")
        return 1

    try:
        time.sleep(2.0)
        ser.reset_input_buffer()

        if args.command == "list":
            do_list(ser)
        elif args.command == "format":
            do_format(ser)
        elif args.command == "upload":
            remote = args.remote or os.path.basename(args.local)
            if "/" in remote or ".." in remote:
                raise RuntimeError("remote name must be a plain filename")
            do_upload(ser, args.local, remote)
    except (TimeoutError, RuntimeError) as exc:
        print("\nError: %s" % exc)
        return 1
    finally:
        ser.close()

    return 0


if __name__ == "__main__":
    sys.exit(main())
