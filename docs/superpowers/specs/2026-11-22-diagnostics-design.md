# CataBox Diagnostics Design

> Status: approved 2026-11-22 (user chose approach A).
> Scope: bring-up tooling for the v1 firmware. Does not change playback behaviour.

## 1. Intent

The production firmware waits for the SD card before it polls NFC or touches
audio. One broken subsystem therefore blocks every other test. During hardware
bring-up we need to prove each part independently: reader, card, amplifier.

This adds three diagnostic firmware builds and keeps the normal build usable
when the SD card is missing.

## 2. Modes

| PlatformIO env | What it proves | Needs |
|---|---|---|
| `diag_i2c` | scans the I2C bus; PN532 must appear at `0x24` | PN532 only |
| `diag_nfc` | PN532 init, firmware version, every UID tapped | PN532 only |
| `diag_sd` | SPI mount, card type/size, `/stories` listing | SD module + card |
| `diag_audio` | SD mount, MP3 decode, I2S amp, speaker | SD + amp + speaker |
| `esp32dev` (normal) | Full app | everything |

Commands:

```bash
pio run -e diag_nfc -t upload && pio device monitor -b 115200
pio run -e diag_sd -t upload && pio device monitor -b 115200
pio run -e diag_audio -t upload && pio device monitor -b 115200
```

All diagnostics log at 115200 and never require the other subsystems.

## 3. Normal-build change

When the SD card is not mounted, the normal build still:

- retries the mount every 5 s,
- polls the PN532 and prints `UID (no SD): ...` when a tag is tapped,
- blinks the status LED slowly.

It still does not start the app until the SD card mounts. This lets the reader
be verified while the card problem is being fixed.

## 4. Serial card writer

`diag_sd` also turns the box into a card writer, so bring-up does not need a
separate card reader:

- `l` lists `/stories`;
- `f` mounts with `format_if_empty = true` (formats a blank/exFAT card), then
  creates `/stories`;
- `u <name> <bytes>` waits for `READY`, receives exactly `<bytes>` of binary
  data over serial, writes `/stories/<name>`, and replies `OK <written> <sum>`.

`tools/sdcard.py` wraps those commands (`list`, `format`, `upload`). Speed at
115200 baud is about 11 KB/s; the script prints progress. The protocol is
diagnostic-only and is not compiled into the normal firmware.

## 5. SD mount fallback

`mountSd()` tries 4 MHz, then 1 MHz, then 400 kHz, calling `SD.end()` between
attempts, and prints the speed that worked. Marginal breadboard wiring often
fails at 4 MHz and works at 1 MHz or lower. The production app and all
diagnostics use it.

## 6. Code structure

```
firmware/src/
  arduino_sd.h/.cpp      # mountSd() speed fallback
  diagnostics.h          # diagNfcBegin/Loop, diagSdBegin/Loop, diagAudioBegin/Loop
  diag_i2c.cpp           # compiled only with -DCATABOX_DIAG_I2C
  diag_nfc.cpp           # compiled only with -DCATABOX_DIAG_NFC
  diag_sd.cpp            # compiled only with -DCATABOX_DIAG_SD; also serial uploader
  diag_audio.cpp         # compiled only with -DCATABOX_DIAG_AUDIO
  main.cpp               # dispatch on the compile flag; normal path otherwise
```

`diag_audio` looks for `/stories/diag.mp3` first, then the first `*.mp3` in
`/stories`, and prints position once a second. `media::hasMp3Extension()` is
host-tested.

## 7. Non-goals

- No serial command shell in the normal build.
- No changes to the tag state machine, bookmarks, or buttons.
- No new hardware.

## 8. Acceptance

1. `diag_i2c` reports `0x24`; `diag_nfc` prints `PN532 ready` and UIDs with no SD card present.
2. `diag_sd` prints mount speed, card type/size, and each file in `/stories`; `f` formats a blank/exFAT card; `u` accepts a file over serial.
3. `diag_audio` plays a test MP3 and prints position/end/error.
4. Normal build prints UIDs while the SD card is missing, then starts the app
   once the card mounts.
5. All three envs build; the normal env still builds; host tests stay green.
