# Flashing a model to Grove Vision AI V2 — PC only (no mobile, no XIAO pin link)

The production camera web-server stream needs a real `.tflite` on Grove. If
you only need to smoke-test XIAO's direct MJPEG endpoint without Grove hardware,
build `xiao_esp32s3_camera_web_fake` and run
`tools/check_grove_streaming.ps1 -Host <device-ip>` instead. That fake target
injects synthetic JPEG frames into `/stream/frame`; it does not emulate the
stock browser UI Start command.

Both XIAO and Grove are plugged into the PC over USB. They are NOT pin-wired
to each other. This is the **supported** setup for the SenseCraft Web Toolkit
and is what the Seeed wiki documents. The mobile app is not required.

The pretrained-model upload via the Web Toolkit talks to Grove **directly**
over its own USB-C (COM11 / CH343). The XIAO is irrelevant for this step.

## The exact sequence that works

1. **Driver — install the right CH343 driver.** Multiple users reported the
   "model registers but binary never lands" symptom until they replaced the
   driver:
   - Run `CH343SER.EXE` from
     <https://files.seeedstudio.com/wiki/grove-vision-ai-v2/res/CH343SER.EXE>
   - Reboot Windows. Confirm Grove enumerates as `USB-Enhanced-SERIAL CH343`
     in Device Manager (VID 1A86 / PID 55D3).
2. **Browser — Chrome or Edge only** (Web Serial API). Firefox / Safari will
   appear to work but cannot complete the binary transfer.
3. **Free the COM port.** Close any other tool holding COM11:
   - PlatformIO Serial Monitor
   - Arduino IDE
   - any `python` / `pwsh` `SerialPort` session
   - the firmware-side AT proxy on the XIAO is fine — it talks to its own
     COM12, not Grove's COM11.
4. **Open the toolkit:** <https://seeed-studio.github.io/SenseCraft-Web-Toolkit/#/setup/process>
5. **Top-left dropdown — choose `Grove Vision AI (WE2)`** (NOT XIAO ESP32S3).
   This is the most common mistake: leaving it on XIAO causes the toolkit to
   only update the descriptor blob.
6. Click **Connect**, in the Chrome picker pick the CH343 port (COM11), click
   **Confirm**.
7. In the **Upload AI Model** panel pick a preset (e.g. `Person Detection`)
   or upload a custom `*_vela.tflite`. Click **Send**.
8. **Do not switch browser tabs, do not lock the screen, do not unplug.**
   The wiki explicitly warns this aborts the flash and only the metadata
   blob ends up on flash — exactly the symptom you've been hitting. Keep
   the tab focused for the full 1–2 minutes.
9. When the progress bar reaches 100 % and the toolkit shows the live preview,
   the binary is on flash.

## If step 5 dropdown does not show "Grove Vision AI (WE2)"

That option only appears once the device responds to AT over the **Single
Serial** transport. Two recovery paths:

- **Power-cycle path:** unplug Grove → hold BOOT (smaller button next to
  USB-C) → plug Grove into PC while still holding BOOT → release BOOT after
  ~2 s. This forces Himax DFU, after which the toolkit always offers the
  WE2 option. (Wiki: "Boot / Reset / Flashed Driver — Method 1".)
- **Reset path:** with Grove plugged in, press BOOT and then immediately
  press RESET. (Wiki: "Method 2".)

## After-flash verification (PC-only, no XIAO needed)

Run `python tools\grove_flash_check.py COM11` from the venv. It will:

1. Open COM11 at 921600 8-N-1 (CH343 native).
2. Send `AT+ID?` then `AT+MODELS?`.
3. Parse the JSON reply and print the model size.
4. Exit non-zero if `size == 0` (i.e. only descriptor was written).

## Things known to BREAK the upload (collected from forum threads)

- Running the `we2_iic_bootloader_recover` Arduino sketch — multiple users
  found it leaves Grove in a state where SenseCraft uploads only register
  metadata afterwards. **Do not run it** unless Grove is already bricked.
- Using a USB-A → USB-C cable that is power-only. Use the data-capable
  USB-C cable.
- Plugging Grove into a USB hub during the upload. Plug it directly into
  the PC.
- Switching browser tabs / opening DevTools network tab during upload.
- Selecting the wrong board in the toolkit's top-left dropdown.

## What the XIAO is doing in this picture

Nothing. The XIAO firmware on COM12 is unrelated to model flashing. It
will start streaming JPEGs the moment Grove reports `MODELS?` size > 0
and `SENSOR=1,1,2` returns code:0.
