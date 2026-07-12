# ESP32-S3-LCD-1.47B Starter Project

A beginner-friendly (and hopefully reference-worthy for experienced folks
too) starting point for developing your own interface/app for the
**Waveshare ESP32-S3-LCD-1.47 inch LCD Display Development Board — Type B**.

This repo includes:
- A first-boot walkthrough: what to expect before and after you plug the
  board in, and how to confirm your Mac sees it.
- A ready-to-build PlatformIO project (`src/main.cpp`) that exercises every
  major piece of onboard hardware — display, IMU, Wi-Fi, RGB LED, button.
- Small standalone examples in `examples/` for learning one peripheral at a
  time.

No prior ESP32 or embedded experience assumed.

---

## Table of contents

1. [Hardware specs](#hardware-specs)
2. [What's in the box](#whats-in-the-box)
3. [Before you plug it in](#before-you-plug-it-in)
4. [Plugging it in — what to expect](#plugging-it-in--what-to-expect)
5. [Confirming macOS sees it](#confirming-macos-sees-it)
6. [Software setup](#software-setup)
7. [Build & upload the starter app](#build--upload-the-starter-app)
8. [Project tour](#project-tour)
9. [Troubleshooting](#troubleshooting)
10. [Next steps](#next-steps)
11. [Contributing](#contributing)

---

## Hardware specs

| Category | Spec |
|---|---|
| MCU | ESP32-S3R8 — Xtensa 32-bit LX7 **dual-core**, up to **240 MHz** |
| RAM | 512 KB SRAM (on-chip) + **8 MB PSRAM** (octal) |
| ROM | 384 KB |
| Flash | **16 MB** |
| Display | 1.47" IPS LCD, **172×320** resolution, **262K colors**, ST7789 driver, 3-wire SPI |
| Wireless | 2.4 GHz Wi-Fi 802.11 b/g/n, **Bluetooth 5 (BLE)**, onboard antenna |
| IMU | QMI8658 6-axis (3-axis accelerometer + 3-axis gyroscope), I2C |
| RGB LED | Onboard addressable LED behind a clear acrylic panel *(Type B only — the touch variant of this board has a touch controller here instead)* |
| Storage | microSD / TF card slot (4-bit SDMMC) |
| USB | USB Type-C, via onboard **CH343P** USB-to-UART bridge (power, flashing, serial logs) |
| Battery | JST connector for 3.7V Li-Po, onboard charge management |
| Buttons | BOOT and RESET |

See [`include/pins_config.h`](include/pins_config.h) for the full GPIO pin
map used by this project (with notes on how confident each pin's source is).

> **A note on accuracy:** Waveshare doesn't publish a single authoritative
> pinout table for every revision of this board, so the pin map here was
> cross-referenced across Waveshare's official listing, their wiki, and
> independent community projects targeting the same panel/IMU combo. If you
> find a discrepancy against your unit's schematic, please open an issue or
> PR — see [Contributing](#contributing).

---

## What's in the box

For the listing this repo was written against ("with pre-soldered Header"):

- The board itself, with pin headers **already soldered on**
- Possibly a spare/extra set of headers (varies by seller bundle)
- **Usually no USB-C cable** — check your specific listing; most Waveshare
  boards don't include one, so have a USB-C **data** cable ready (not a
  charge-only cable — those are missing the data wires and the board simply
  won't be detected by your Mac)

---

## Before you plug it in

With the board powered off, out of the antistatic bag:

- The **screen is blank/black** — nothing lights up, nothing draws power,
  this is expected.
- There's usually a **thin protective plastic film** over the display
  (and sometimes the acrylic RGB LED panel). Peel it off whenever you like —
  it's just shipping protection, not functional.
- No LEDs are lit, no buzzing, nothing warm — it's inert until it gets
  power over USB (or a battery, if you've wired one in).
- Inspect the header pins for anything bent from shipping before you plug
  it into anything.

---

## Plugging it in — what to expect

1. **Connect a USB-C data cable** from the board to your Mac.
2. The board powers on **immediately** — no power switch to flip.
3. Most units ship from the factory with a small pre-flashed **demo/test
   program**. What exactly it shows can vary by batch — commonly this looks
   like a splash graphic, a scrolling test pattern, live IMU readouts, or
   the RGB LED cycling colors. **Don't worry if what you see doesn't match
   any description you've read** — this factory firmware only exists to
   prove the board isn't dead on arrival, and the first time you upload
   this repo's project, it gets completely overwritten.
4. macOS itself usually gives **no popup or notification** for a bare USB
   serial device like this — that's normal. Don't expect a "new device
   connected" banner. The way to actually confirm it's detected is the
   Terminal check below.

---

## Confirming macOS sees it

The board's USB-C port is wired through a **CH343P** USB-to-UART chip (not
the ESP32-S3's native USB peripheral), so your Mac needs to enumerate that
chip as a serial device.

**1. Open Terminal and check for the device before and after plugging in:**

```sh
ls /dev/cu.*
```

Plug the board in, then run it again. You're looking for a new entry that
appears, typically named something like:

```
/dev/cu.usbserial-XXXXXXXX
/dev/cu.wchusbserialXXXXXXXX
```

- **If a new device shows up:** you're done — modern macOS (Sonoma/Sequoia)
  often has native support for CH34x-family chips and no driver install is
  needed.
- **If nothing new appears:**
  1. Double-check the cable is a **data** cable, not charge-only.
  2. Try a different USB-C port (and a USB-C-to-A adapter if you're on an
     older Mac with only USB-A ports, using a known-good cable).
  3. Install the official WCH CH343/CH34x macOS driver, then reboot.
  4. During/after driver installation, macOS will likely block the kernel
     extension until you approve it in
     **System Settings → Privacy & Security → Security**, near the bottom
     of the page ("System software from developer 'X' was blocked"). Click
     **Allow**, then reboot again.
  5. Re-run `ls /dev/cu.*` and confirm the device reappears.

You can also check **System Information → USB** (hold ⌥ and click the
Apple menu → System Information) for an entry referencing a USB-serial
chip when the board is plugged in.

---

## Software setup

This project uses **[PlatformIO](https://platformio.org/)** with the
Arduino framework — it manages the ESP32 toolchain and libraries for you,
so there's no separate "board package" install step like the Arduino IDE
requires.

1. Install **[Visual Studio Code](https://code.visualstudio.com/)**.  You can also use whatever is your preferred coding assistant combo i.e. Cursor + Claude or Codex, obviously.
2. In VS Code, open the Extensions panel (`Cmd+Shift+X`) and install
   **"PlatformIO IDE"**. Wait for it to finish installing its own toolchain
   (a progress bar appears bottom-left; this can take a few minutes the
   first time).
3. Clone or download this repo, then in VS Code: **File → Open Folder…**
   and select the `esp32project` folder.
4. PlatformIO will detect `platformio.ini` and set the project up
   automatically — you'll see a PlatformIO icon (alien head) appear in the
   left sidebar.

---

## Build & upload the starter app

With the board plugged in and the project open in VS Code:

1. Click the **PlatformIO icon** in the sidebar → **PROJECT TASKS** →
   `waveshare-esp32s3-lcd-1_47b` → **General** → **Upload**
   (or just press the **→ (arrow)** icon in the bottom status bar).
2. PlatformIO will download the toolchain/libraries on first run, compile,
   then flash the board. You'll see the CH343 device connect/disconnect
   briefly during flashing — that's normal.
3. Once flashing finishes, the board resets and runs the app. You should
   see the **"Board Info"** screen.
4. Press the physical **BOOT** button on the board to cycle through the
   four demo screens (Board Info → IMU Reader → Wi-Fi Scanner → RGB
   Playground → back to Board Info).
5. Open the **Serial Monitor** (plug icon in the status bar, or PROJECT
   TASKS → **Monitor**) to see log output at 115200 baud.

Command-line equivalent, if you prefer a terminal:

```sh
pio run --target upload
pio device monitor
```

---

## Project tour

```
esp32project/
├── platformio.ini        # Build config: board, memory layout, dependencies
├── include/
│   └── pins_config.h      # Every GPIO pin used, in one place, heavily commented
├── src/
│   └── main.cpp           # The starter app (all 4 demo screens)
└── examples/               # Copy any of these into src/main.cpp to try in isolation
    ├── 01_hello_display.cpp
    ├── 02_blink_rgb_led.cpp
    ├── 03_button_input.cpp
    ├── 04_wifi_scan_serial.cpp
    └── 05_bitcoin_block_clock.cpp
```

`src/main.cpp` is deliberately written **without a GUI framework** (no
LVGL) — it draws directly with
[LovyanGFX](https://github.com/lovyan03/LovyanGFX) so the full flow (SPI
setup → I2C reads → Wi-Fi calls → drawing) stays visible in one file
instead of being hidden behind widget abstractions. Once you're
comfortable with the basics, LVGL is a natural next step for anything more
than a few screens — see [Next steps](#next-steps).

---
### Which example should I start with?

If you're new to ESP32 development, try the examples in this order:

1. `01_hello_display.cpp` — confirm the display is working and learn the basic drawing flow.
2. `02_blink_rgb_led.cpp` — control the onboard RGB LED.
3. `03_button_input.cpp` — read input from the physical BOOT button.
4. `04_wifi_scan_serial.cpp` — scan nearby Wi-Fi networks and print the results to the Serial Monitor.
5. `05_bitcoin_block_clock.cpp` — connect to mempool.space over HTTPS and
   display the current block height, next halving countdown, and fee rates.
   A working starting point for any Bitcoin-aware device app.

To try an example, copy its contents into `src/main.cpp`, build the project, and upload it to the board.

Starting with the display example is recommended because it gives immediate visual confirmation that the project, board, and upload process are working correctly.

## Troubleshooting

| Symptom | Try this |
|---|---|
| Board not detected on Mac | See [Confirming macOS sees it](#confirming-macos-sees-it) above |
| Upload fails / times out | Hold **BOOT**, tap **RESET**, release **BOOT** right after — forces download mode, then retry upload |
| Screen stays blank after upload | Check `pins_config.h` SPI pins against your revision; try toggling `cfg.invert` in the `LGFX` class in `src/main.cpp` |
| Colors look inverted or shifted sideways | Adjust `cfg.invert` and `LCD_COL_OFFSET`/`LCD_ROW_OFFSET` in `pins_config.h` |
| "IMU: not found" on Board Info screen | Confirm `PIN_IMU_SDA`/`PIN_IMU_SCL` in `pins_config.h` against your board revision; some early units may differ |
| Build fails on first run | Make sure PlatformIO finished its initial toolchain download (check the bottom status bar / PlatformIO Home for progress) before building |

---

## Next steps

Once you're comfortable with this starter app, natural directions to take
it:

- Swap the hand-rolled QMI8658 reads for a full driver (calibrated units,
  motion interrupts) — e.g. `hideakitai/QMI8658`.
- Add [LVGL](https://lvgl.io/) for real widgets (buttons, sliders, menus)
  instead of hand-drawn text screens.
- Use the microSD slot to log IMU data or load images/fonts.
- Extend `05_bitcoin_block_clock.cpp` with price data, a Lightning invoice
  QR code (see the `qrcode` library for ESP32), or BLE push to a companion phone app.
- Wire up a 3.7V Li-Po battery to the JST connector and add deep-sleep
  power management for a portable build.

---

## Contributing

New contributors — including total beginners — are welcome. See
[CONTRIBUTING.md](CONTRIBUTING.md) for how to add examples, fix pin
mappings, or improve these docs.
