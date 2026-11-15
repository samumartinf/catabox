# CataBox v1 — Minimal Custom Firmware Design

> Status: approved 2026-11-15. Supersedes ADR-005 (ESPuino) for v1.
> Date: 2026-11-15

## 1. Intent

A child places a figurine on the box; the box plays that figurine's story. Lift it
and the story pauses, holding the spot. Put it back and the story continues.
Two buttons: **pause/resume** and **restart from the beginning**.

No WiFi, no web UI, no app, no screen. Content is a folder of MP3s on a microSD
card. This is the smallest thing that behaves like a Toniebox and that we can
debug ourselves.

**Why this supersedes ESPuino:** ESPuino is a large, config-driven codebase. We
want a program small enough to hold in our heads, with exactly two buttons and
no network. The trade-off we accept: no web upload, no FTP, no OTA — stories are
added by pulling the SD card.

## 2. Goals

- Tap figurine → its story plays.
- Lift figurine → pause within ~1 s, remember position.
- Replace figurine → resume where it stopped.
- Pause button toggles pause/resume.
- Restart button jumps to 0:00 of the current story and plays.
- Position survives a power cycle (per figurine bookmark).
- All logic understandable by one person in an evening.

## 3. Non-goals (v1)

- WiFi, web UI, FTP, OTA, MQTT, Bluetooth.
- Multiple tracks / chapters per figurine (one MP3 per figurine).
- Play modes, shuffle, sleep timer, lock cards.
- NeoPixel ring, battery, charging circuit, enclosure CAD.
- Volume control (fixed, moderate level).
- Reading original Tonies or writing tags.

## 4. Hardware

| Part | v1 role | Notes |
|---|---|---|
| ESP32 devkit (WROOM-32 presumed) | brain | PSRAM **not** required; confirm board model before wiring |
| PN532 NFC module | reads figurine UID | I2C mode; confirm DIP switch setting on the actual board |
| MAX98357A I2S amp | audio out | 5 V supply, 3.3 V logic |
| microSD SPI module + card (FAT32) | audio storage | Class 10, 8–32 GB |
| 2× 30 mm arcade buttons | pause/resume, restart | wired to GND, internal pull-ups |
| Speaker 4–8 Ω 3 W | sound | |
| 5 V 2 A USB supply | power | 470 µF bulk cap near amp recommended |
| NTAG213/215 stickers + magnets | figurines | physical only; magnet detuning test early |

No custom PCB. Breadboard for bring-up, perfboard for the final build.

## 5. Behaviour specification

### 5.1 State model

Three inputs drive everything:

- `loadedUID` — the story currently open (or none).
- `tagPresent` — the loaded figurine is on the box.
- `userPaused` — the pause button was pressed and has not been undone.

Audio plays **iff** a story is loaded, its tag is present, and it is not
user-paused. Pause reasons are independent flags rather than one enum, so a
parent pressing pause is never undone by a child lifting and replacing a
figurine.

Two latches prevent retry storms while a tag sits on the box:

- `finishedUID` — track ended; ignore automatic retries until that tag is removed
  or the restart button is pressed.
- `failedUID` — file missing, empty/not-MP3, or decode error; ignore automatic
  retries until tag removal or restart.

Both latches clear when the tag becomes absent.

### 5.2 Tag presence and multiple tags

Poll the PN532 every 100 ms. A tag counts as **present** when a read succeeds;
**absent** only after 500 ms with no successful read (debounce against dropouts).

Only one figurine is ever "the" figurine:

- The PN532 library reports one tag per poll. While the current tag is seen at
  least once every 500 ms, other readings are ignored — the current story wins.
- When the current tag has not been seen for 500 ms, the newest reading becomes
  the current tag.
- At boot/idle, the first tag read becomes the current tag.

This is a best-effort version of "current wins": with two tags stacked, a reader
that alternates between them keeps the current story; a reader that only reports
the top tag switches after 500 ms. Stacking is documented as unsupported.

UIDs of 4 or 7 bytes are accepted. Anything else is logged and ignored.

### 5.3 Transitions

| Event | Condition | Action |
|---|---|---|
| Tag present | no story loaded, no latch for that UID, file exists | open file, seek to bookmark, clear `userPaused`, play |
| Tag present | file missing | log, blink 3×, set `failedUID` |
| Tag absent | story loaded | pause, save bookmark |
| Same tag returns | story loaded, not user-paused | reopen at the saved byte and resume |
| Same tag returns | story loaded, user-paused | stay paused |
| Different tag placed | other tag absent | save bookmark, stop old, load new |
| Different tag placed | loaded tag still present | ignore (current wins) |
| Pause button | story loaded and tag present | toggle `userPaused`; save bookmark when pausing |
| Pause button | no tag present | ignore |
| Restart button | story loaded and tag present, including after finish/error | clear latches, play from 0, clear bookmark on success, `userPaused = false` |
| Restart button | no tag present | ignore |
| Track ends | — | stop, clear bookmark, set `finishedUID` |
| Decode error | — | stop, keep bookmark, set `failedUID` |

Restart requires the figurine on the box. A toddler cannot make the box play to
an empty room. Restart is also the recovery path for a finished story or a
broken file: it clears the latch and retries from 0:00.

### 5.4 Bookmarks

- Stored in NVS (`Preferences`), namespace `catabox`, key = UID hex (≤14 chars).
- Values are **byte offsets** into the MP3, since that is what the decoder can
  seek to; ±2 s accuracy is acceptable.
- Saved every 10 s while playing, and on pause, lift, track change, and restart
  (restart clears).
- Cleared when the track finishes.
- Kept on decode error, so a transient glitch does not lose the place; the
  restart button is the escape if a file is permanently broken.
- On start: resume only if the bookmark is at least 8 KB in and at least 8 KB
  before the end of the file; otherwise start at 0.
- Writes are skipped if the position has not advanced ≥ 32 KB.

### 5.5 Buttons

- 40 ms debounce; act on release.
- One action per press (no long-press features in v1).

### 5.6 Audio and volume

- MP3, 44.1 kHz, mono or stereo, 96–160 kbps, constant bitrate preferred.
- Fixed volume set in `config.h` (default ~40% of library range); cap for toddler
  ears. No runtime volume control.
- I2S via `ESP8266Audio` (`AudioGeneratorMP3` + `AudioOutputI2S`). Chosen because
  it decodes MP3 on a plain ESP32 without PSRAM; byte-offset seek is enough for
  bookmarks.

### 5.7 Boot

A figurine already on the box at power-on counts as placed: after a 1 s boot
delay, the box loads it and resumes from its bookmark (or 0:00). Latches are
cleared at boot.

## 6. Content and tag mapping

- Stories live in `/stories/` on the SD card.
- Filename is the tag UID in uppercase hex with no separators:
  `/stories/04A3B2C1D4E580.mp3` (7-byte NTAG213 → 14 chars; 4-byte tags → 8 chars).
- Workflow: tap a new tag → serial prints `UID: 04A3B2C1D4E580` → rename the MP3
  to that → copy to `/stories/`.
- Missing file is a first-class, handled error, not a crash.
- This format leaves room to add `/stories/<UID>/01.mp3` multi-track support
  later without changing the tag registry.

## 7. Pin map (provisional — confirm against the actual board)

| Function | ESP32 | Module |
|---|---|---|
| I2S BCLK | GPIO 27 | MAX98357A BCLK |
| I2S LRC | GPIO 26 | MAX98357A LRC |
| I2S DIN | GPIO 25 | MAX98357A DIN |
| I2C SDA | GPIO 21 | PN532 SDA |
| I2C SCL | GPIO 22 | PN532 SCL |
| SD CS | GPIO 5 | SD module CS |
| SD SCK | GPIO 18 | SD module SCK |
| SD MISO | GPIO 19 | SD module MISO |
| SD MOSI | GPIO 23 | SD module MOSI |
| Pause button | GPIO 32 | button → GND |
| Restart button | GPIO 33 | button → GND |
| Status LED | GPIO 2 | onboard LED (error/status only) |

Deliberately avoided: GPIO 6–11 (flash) and 34–39 (input-only). GPIO 2 (LED) and
GPIO 5 (SD CS) are strapping pins, but both idle high at boot; the SD module’s CS
pull-up keeps GPIO 5 high. PN532 is I2C, so it does not share the SD SPI bus.

Power: all modules 3.3 V logic; MAX98357A VIN on 5 V; common ground; bulk
capacitor across amp VIN/GND.

## 8. Code structure

```
firmware/
  platformio.ini          # env:esp32dev, Arduino framework, lib deps
  src/
    config.h              # pins, volume, timings, paths
    media.h/.cpp          # MP3 signature + clean-end classification
    main.cpp              # setup/loop + state machine
    nfc.h/.cpp            # PN532 init, UID read, multi-tag selection, presence
    player.h/.cpp         # ESP32-audioI2S wrapper, seek, end detection
    controls.h/.cpp       # debounced buttons → events
    bookmarks.h/.cpp      # NVS position load/save/clear
  test/
    test_logic/           # host tests for pure helpers (UID→path, seek rule, selection)
  README.md               # build, flash, SD layout, tag workflow
```

Library dependencies:

- `earlephilhower/ESP8266Audio` — MP3 decode + I2S + SD source.
- `adafruit/Adafruit PN532` — I2C tag reads.

The state machine keeps hardware calls behind `nfc` / `player` / `controls`
interfaces so the transition logic can be host-tested without a board.

## 9. Error handling

| Failure | Behaviour |
|---|---|
| SD mount fails | serial message + slow LED blink; retry every 5 s |
| Tag read fails / no tag | treated as absent; no error spam |
| MP3 missing | serial + 3 blinks; `failedUID` until tag replaced or restart |
| MP3 empty / not an MP3 | rejected at open, same `failedUID` path |
| MP3 decode error | stop, serial, keep bookmark, `failedUID` until replaced or restart |
| I2S/amp not wired | boot continues; serial warns on first playback attempt |

Everything is logged at 115200 baud. No watchdog resets expected; loop is
non-blocking (no `delay()` longer than 10 ms except during boot).

## 10. Acceptance criteria

1. Boots, mounts SD, prints firmware version and UID of any tag placed.
2. Tag placed → correct MP3 plays within ~1 s.
3. Tag lifted → audio pauses within ~1 s; returned → resumes at same spot.
4. Pause button toggles; restart returns to 0:00 and plays.
5. A second figurine swaps stories cleanly once the first is lifted.
6. Power cycle with the figurine still on the box → resumes from its bookmark.
7. Story end → silence; leaving the tag in place does not replay it.
8. Missing MP3 → handled error, no reboot, no crash.
9. Buttons and NFC do not interfere with audio (no clicks/dropouts).

## 11. Risks and open questions

- **PN532 mode:** I2C assumed; DIP switches must be set accordingly. If the module
  is HSU/UART-only in practice, wiring and library calls change.
- **Board identity:** pin map assumes a standard 30/38-pin ESP32 devkit. Confirm
  the actual board before soldering.
- **NFC through PLA + magnets:** detuning can reduce range. Test with the printed
  base as early as possible; move the tag toward the top surface if needed.
- **MP3 seek granularity:** seeking is by byte offset and resyncs on the next
  frame; ±2 s audible accuracy.
- **NVS wear:** 10 s saves are acceptable but can be relaxed to 30 s if desired.
- **PSRAM:** not required; the chosen decoder runs on a plain WROOM.

## 12. Deliberately left to the builder

- Exact PlatformIO library versions.
- Whether to add a small 3D-printed button bezel / mounting scheme.
- Final volume constant after listening test.
- Whether to keep the status LED beyond bring-up.
- Enclosure and figurine CAD (separate workstream).

## 13. Calls made during review (check these while reading)

1. **Pause flags, not one state.** `userPaused` survives lifting, so a parent's
   pause is not undone by a child re-placing the figurine.
2. **Current tag wins (best effort).** Stacking a second figurine does not switch
   stories while the first tag is still being reported; see §5.2 for the limit
   of a single-target reader.
3. **Restart needs the tag.** It cannot start audio with nothing on the box.
4. **Boot counts as placement.** Figurine on the box at power-on resumes after
   1 s (approved).
5. **Absence debounce stays at 500 ms**, so pause lands within ~1 s rather than
   0.5 s. Reliability through PLA beats the tighter number.
6. **Only 4- and 7-byte UIDs accepted**; anything else is logged and ignored.
7. **Decode errors keep the bookmark** and latch until the tag is removed; the
   restart button is the escape from a permanently broken file.
