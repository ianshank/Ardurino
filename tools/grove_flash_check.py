"""Verify a model has actually been flashed to a Grove Vision AI V2.

Usage:
    python tools/grove_flash_check.py [COMxx]

Default port is COM11. Exits 0 if MODELS? returns size > 0, 2 otherwise.
Requires pyserial: pip install pyserial
"""
from __future__ import annotations

import json
import sys
import time

import serial  # type: ignore[import-untyped]

DEFAULT_PORT = "COM11"
BAUD = 921600


def send(ser: serial.Serial, cmd: str, wait: float = 0.6) -> str:
    ser.reset_input_buffer()
    ser.write((cmd + "\r").encode("ascii"))
    ser.flush()
    time.sleep(wait)
    return ser.read(ser.in_waiting or 1).decode("utf-8", errors="replace")


def extract_json(blob: str, name: str) -> dict | list | None:
    # Replies look like:  \r{"type":0,"name":"MODELS?","code":0,"data":[...]}\n
    for line in blob.splitlines():
        line = line.strip().lstrip("\x00")
        if not line.startswith("{"):
            continue
        try:
            obj = json.loads(line)
        except json.JSONDecodeError:
            continue
        if obj.get("name", "").rstrip("?") == name.rstrip("?"):
            return obj.get("data")
    return None


def main() -> int:
    port = sys.argv[1] if len(sys.argv) > 1 else DEFAULT_PORT
    print(f"Opening {port} @ {BAUD}...")
    try:
        ser = serial.Serial(port, BAUD, timeout=1)
    except serial.SerialException as exc:
        print(f"  cannot open: {exc}")
        print("  -> close any other app holding the port (PlatformIO monitor,")
        print("     SenseCraft tab, Arduino IDE) then re-run.")
        return 3

    with ser:
        # drain any boot chatter
        time.sleep(0.2)
        ser.reset_input_buffer()

        ident = send(ser, "AT+ID?")
        print("AT+ID?  ->", ident.strip() or "(no reply)")

        raw = send(ser, "AT+MODELS?", wait=0.8)
        data = extract_json(raw, "MODELS?")
        if data is None:
            print("AT+MODELS? returned no JSON. Raw:")
            print(raw)
            return 2

        print("AT+MODELS? ->", json.dumps(data, indent=2))
        sizes = [m.get("size", 0) for m in data] if isinstance(data, list) else []
        max_size = max(sizes) if sizes else 0
        print(f"\nLargest model size on flash: {max_size} bytes")

        if max_size <= 0:
            print("FAIL: only the descriptor is written; binary did not land.")
            print("Re-flash via SenseCraft Web Toolkit and keep the tab focused")
            print("for the full upload. See docs/grove_model_flash_pc.md.")
            return 2

        print("OK: a model binary is present. Stream should now produce JPEGs.")
        return 0


if __name__ == "__main__":
    sys.exit(main())
