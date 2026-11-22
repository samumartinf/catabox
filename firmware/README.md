# CataBox firmware (v1)

Minimal ESP32 firmware: place a figurine on the box, its story plays. Lift it and
the story pauses, remembering the spot. Two buttons: pause/resume and restart.

No WiFi, no web UI, no app. Stories are MP3s on the microSD card.

- Design: [`../docs/superpowers/specs/2026-11-15-catabox-v1-firmware-design.md`](../docs/superpowers/specs/2026-11-15-catabox-v1-firmware-design.md)
- Wiring diagram: [`../docs/wiring/catabox-wiring.svg`](../docs/wiring/catabox-wiring.svg)
- Implementation plan: [`../docs/superpowers/plans/2026-11-15-catabox-v1-firmware.md`](../docs/superpowers/plans/2026-11-15-catabox-v1-firmware.md)

## Hardware

| Part | Notes |
|---|---|
| ESP32 devkit (WROOM-32 is fine) | no PSRAM needed |
| PN532 NFC module | DIP switches set to **I2C**, powered from **3.3 V** |
| MAX98357A I2S amp | VIN on 5 V, speaker on the output terminals |
| microSD SPI module + card | FAT32, Class 10 |
| 2× 30 mm arcade buttons | pause (GPIO 32), restart (GPIO 33), other side to GND |
| Speaker 4–8 Ω, 3 W | |

Pin map and power notes are in the [wiring diagram](../docs/wiring/catabox-wiring.svg).

## Build and flash

PlatformIO with the Arduino framework:

```bash
cd firmware
pio run -e esp32dev                 # build
pio run -e esp32dev -t upload       # flash over USB
pio device monitor -b 115200        # serial log
```

On this Mac, if `pio` reports a missing `tool-scons` package for
`darwin_x86_64` on the native tests, use the arm64 install:

```bash
/Users/samu/.venvs/platformio/bin/pio test -e native
/Users/samu/.venvs/platformio/bin/pio run -e esp32dev
```

## Diagnostics

Bring-up builds that test one subsystem at a time, so a broken SD card does not
hide the reader:

| Env | Tests | Needs |
|---|---|---|
| `diag_i2c` | scans the bus; PN532 should appear at `0x24` | PN532 only |
| `diag_nfc` | PN532 init, firmware version, every UID tapped | PN532 only |
| `diag_sd` | mount speed, card type/size, `/stories` listing | SD module + card |
| `diag_audio` | SD mount, MP3 decode, I2S amp, speaker | SD + amp + speaker |

```bash
pio run -e diag_i2c -t upload && pio device monitor -b 115200
pio run -e diag_nfc -t upload && pio device monitor -b 115200
pio run -e diag_sd -t upload && pio device monitor -b 115200
pio run -e diag_audio -t upload && pio device monitor -b 115200
```

`diag_audio` plays `/stories/diag.mp3` if present, otherwise the first `.mp3` in
`/stories`. The normal build now also prints `UID (no SD): ...` while the card is
missing, then starts the app once it mounts.

SD mounting tries 4 MHz, 1 MHz and 400 kHz; the serial log prints the speed that
worked.

## Host tests

The state machine and helpers are tested without hardware:

```bash
pio test -e native
```

31 tests cover UID formatting, path building, resume rules, MP3/end
classification, pause/lift/resume, button behaviour, tag swaps, restart after
finish/error, and finish/error latches.

## SD card layout

```
/stories/
  04A3B2C1D4E580.mp3     <- one story per figurine
  DEADBEEF.mp3           <- 4-byte tag example
```

The filename is the tag UID in uppercase hex, no separators.

### Adding a story

1. Format the card FAT32 and create `/stories/`.
2. Flash the box, insert the card, open the serial monitor at 115200.
3. Place the figurine on the reader. The log prints:

   ```
   UID: 04A3B2C1D4E580
   ```

4. Rename the story file to that UID plus `.mp3` and copy it into `/stories/`.
5. Lift and replace the figurine. If the file is missing, the box logs
   `story file missing` and blinks the onboard LED.

Use MP3, 44.1 kHz, mono or stereo, 96–160 kbps, constant bitrate preferred:

```bash
ffmpeg -i story.m4a -codec:a libmp3lame -b:a 128k -ar 44100 -ac 1 \
  -write_xing 0 /path/to/sd/stories/04A3B2C1D4E580.mp3
```

## Controls

| Action | Result |
|---|---|
| Place figurine | Plays that story (from the saved spot, or 0:00) |
| Lift figurine | Pauses within ~1 s and saves the spot |
| Replace same figurine | Resumes from the saved spot |
| Pause button | Pauses/resumes; a manual pause survives lifting the figurine |
| Restart button | Jumps to 0:00 and plays (figurine on the box; works after finish or error) |
| Power on with figurine on box | Starts that story after 1 s |

A finished story does not replay automatically while its figurine stays on the
box; press restart or lift and replace it. A missing or broken file is latched
the same way, so the box does not retry in a loop; press restart to retry it from
0:00. A file that is empty, not an MP3, or unreadable is reported as
`audio open failed` instead of looking like a finished story.

## Behaviour notes

- Bookmarks live in NVS and survive power cycles; values are byte offsets, so
  resume accuracy depends on the MP3 and is roughly ±2 s.
- Only 4-byte and 7-byte UIDs are accepted. Other tags are logged and ignored.
- One figurine at a time is the supported use. Stacking two can confuse the
  reader; the story already playing keeps priority while its tag is still seen.
- Fixed volume is `kVolume` in `lib/catabox_core/config.h` (`0.4` by default).
- The onboard LED (GPIO 2) blinks slowly when the SD card is missing and flashes
  on story errors.

## Troubleshooting

| Symptom | Check |
|---|---|
| `SD mount failed` | FAT32, card seated, CS on GPIO 5, module power (5 V if it has a regulator) |
| `PN532 init failed` | DIP switches on I2C, SDA 21 / SCL 22, 3.3 V, common ground |
| Tag never reads | Tag is NTAG213/215, within a few mm of the coil; try without the printed base |
| No sound | MAX98357A VIN on 5 V, BCLK 27 / LRC 26 / DIN 25, speaker on amp output |
| Sound stutters | use a Class 10 card, 128 kbps CBR MP3, shorter cables |
| Wrong/no story | filename must match the UID exactly, uppercase, `.mp3` |
| `audio open failed` | file is empty, not really an MP3, or unreadable; re-encode it |
| Pauses by itself | NFC dropouts through thick PLA or magnets; move the tag closer to the reader |

## Layout

```
firmware/
  platformio.ini
  lib/catabox_core/     # Arduino-free state machine + helpers (host-tested)
  src/                  # ESP32 adapters: NFC, player, NVS, buttons, main
  test/test_core/       # Unity tests for the core
```
