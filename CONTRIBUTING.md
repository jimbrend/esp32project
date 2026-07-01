# Contributing

This repo exists to be a friendly on-ramp to the Waveshare ESP32-S3-LCD-1.47B
board — whether this is your first microcontroller or your fiftieth. New
contributors are very welcome.

## Ways to help

- **New examples** — got a demo that shows off something the board can do
  (BLE, deep sleep, SD card, a small game)? Add it to `examples/` following
  the style of the existing ones: short, single-purpose, heavily commented.
- **Fixes to `pins_config.h`** — if you've confirmed a pin against the
  official schematic or your own multimeter/continuity testing, a PR with
  the source noted in the commit message is extremely welcome. Several
  values in there are best-effort (see the note at the top of the file).
- **Docs** — clearer explanations, screenshots, or fixes to steps in
  README.md that didn't work on your machine/OS version.
- **Bug reports** — if something in `src/main.cpp` doesn't behave as
  described, open an issue with your OS, PlatformIO version, and what you
  saw vs. expected.

## Ground rules

- Keep examples beginner-readable: prefer clarity over cleverness, and
  comment the *why*, not just the *what*.
- Don't add heavy dependencies (a new graphics/UI framework, etc.) to the
  default build without discussing it in an issue first — the goal is to
  keep `pio run` working with a clean install for a first-timer.
- Test on real hardware before opening a PR where possible, and say so in
  the PR description.

## Getting started

See the "Software Setup" section of [README.md](README.md) — it's the same
workflow you'd use to build the project locally.
