# Oven clock

A wall clock for a Samsung NQ70M7770 wall oven. A 3D-printed cover plate sits over the oven's control-panel glass, held on by magnets, and holds an [Xteink X4 Classic](https://www.xteink.com/) e-ink reader running clock-only firmware.

Print files and photos: [MakerWorld](https://makerworld.com/en/models/3561311-oven-control-panel-cover-with-e-ink-clock).

![The clock face: 11:25 in thin white digits on black](images/clock-face.png)

![The cover plate](images/cover.png)

## What's here

| Folder | What it is |
| --- | --- |
| `plate/` | The cover plate model, current version v5: `oven-plate.scad` (OpenSCAD source) and `oven-plate.stl`. |
| `plate/print/` | **What to print:** `oven-plate-v5-H2C-PLA.gcode.3mf` (H2C, PLA, gyroid 15%), plus the print-orientation files. |
| `plate/versions/` | Earlier versions, v1–v4, with their print files (v3 also has an X1C file, rotated 45° to fit that bed). |
| `firmware/` | The clock firmware: a fork of [papyrix-reader](https://github.com/bigbag/papyrix-reader) by bigbag (MIT), with its full history. |
| `tools/` | `make-clock-font.py`, which turns a TTF into the clock's digit bitmaps. |
| `images/` | Renders of the plate and the clock face. |

## The plate

- Fits the oven's dark control panel (119 mm tall) with 1.5 mm to spare at top and bottom: the plate is 116 mm tall.
- The window lines up with the 90 mm black glass. The magnet slots sit on the metal trim above and below the glass, 1 mm from its edge.
- Magnets go in pairs: one stuck to the oven trim, one glued into the plate.
- The PLA files are for test fits. Print the final plate in PETG or ASA, since it sits on an oven.
- There's no X1C file for v5 yet; the v3 one shows the 45° layout that fits that bed.

## The firmware

Changes from papyrix-reader (all in `firmware/src/apps/` and `firmware/src/main.cpp`):

- Boots straight into the clock app instead of the reader's home screen.
- Shows the time in landscape (power button on the right), 12-hour, US Pacific time with daylight saving time worked out in firmware (`ClockFrame.cpp`).
- Default face "Modern": Inter Display Light digits, white on black. "Modern light" (black on white) and "Segment" (seven-segment digits) can be picked under Face in the clock settings.
- Syncs the time over Wi-Fi each time it starts, then once a day. Between minutes it sleeps, and the reader's real-time clock keeps time.
- Stays awake while plugged into USB, which makes flashing easy.
- The battery icon only appears below 15%.

Wi-Fi networks are set up on the device (Settings › Wi-Fi) and saved on its microSD card.

### Build and flash

PlatformIO needs Python 3.10–3.14.

```bash
cd firmware
pio run -e x4c
esptool --chip esp32s3 --port /dev/cu.usbmodem1101 --baud 921600 write-flash 0x10000 .pio/build/x4c/firmware.bin
```

If the reader is asleep and doesn't show up over USB, take out the microSD card and press power. It stays awake on an error screen and can be flashed. Then put the card back and reset it.

Back up the stock firmware before flashing for the first time (`esptool read-flash 0 0x1000000 backup.bin`).

### Changing the font

```bash
python3 tools/make-clock-font.py path/to/font.ttf firmware/src/apps/fonts/ClockFontInter.h preview.png
```

The script picks the largest size where the widest time still fits the screen.

## Licenses

- Firmware: MIT, from papyrix-reader (see `firmware/LICENSE`).
- Inter font: SIL Open Font License 1.1 (see `firmware/src/apps/fonts/Inter-LICENSE.txt`).
- Plate files and tools: MIT.
