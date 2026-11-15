# Decision Log (ADRs)

Append-only. One entry per decision. Format: context → decision → consequences.

## ADR-001 — Platform: ESP32 (WROOM-32 devkit)
- **Date:** 2026-10-04
- **Context:** Need WiFi-capable MCU (optional future), I2S audio, SD, cheap; Samuel has one.
- **Decision:** ESP32 devkit, Arduino/PlatformIO.
- **Consequences:** Superseded by ADR-005: must be **WROVER (PSRAM)** for ESPuino.

## ADR-002 — Playback fully offline from SD; WiFi only for management
- **Date:** 2026-10-04
- **Decision:** Audio files on microSD; no network at playtime. WiFi for web UI/FTP/OTA.
- **Consequences:** Simpler, more reliable, kid-safe. Content updates via browser or FTP.
- **Status:** unchanged under ESPuino — ESPuino does exactly this.

## ADR-003 — RFID: RC522 default, PN5180 optional upgrade (amended by ADR-008: PN532 on hand)
- **Date:** 2026-10-04
- **Open:** confirm which module is on hand; magnets may detune tag — test early.

## ADR-004 — Controls: arcade buttons + rotary encoder for MVP (superseded by ADR-007 for v1)
- **Date:** 2026-10-04
- **Decision:** 3× 30mm arcade buttons (play/prev/next) + KY-040 encoder for volume; NeoPixel ring for feedback. Tilt/gesture deferred.
- **Consequences:** Matches ESPuino's supported GPIO scheme; no custom input code.

## ADR-005 — Firmware: adopt ESPuino instead of custom firmware ⭐ (superseded by ADR-007)
- **Date:** 2026-10-04
- **Context:** Drafted our own firmware spec (nfc/player/store/ui/power modules) before discovering ESPuino. ESPuino is mature (400+★, 1.4k commits) and already implements the whole feature set: web UI, FTP, audiobook position memory, play modes per tag, modification cards (button lock, sleep timer), NeoPixel support, pre-built firmware.
- **Decision:** Use ESPuino. Start with pre-built firmware; compile only when we need custom GPIO changes (`settings-custom.h`).
- **Consequences:** Weeks of firmware work avoided; our effort shifts to enclosure, figurines, and content — the parts that make it *CataBox*. Hardware constraint: ESP32-WROVER with PSRAM is mandatory.
- **Note:** the figurine characters + family voice stories remain fully custom — that's our differentiator.

## ADR-006 — Enclosure: animal-face design (not rocket)
- **Date:** 2026-10-04
- **Decision:** Owl/bear/fox/cat/puppy design (pick in Phase 3). Flat top with NFC recess (~39mm coil) + magnet recesses; speaker grille as mouth/belly; buttons as paws/ears; NeoPixel ring as eyes/collar.
- **Open:** which animal — Ale's call 😄

## ADR-007 — Firmware: minimal custom build for v1 ⭐ (supersedes ADR-005)
- **Date:** 2026-11-15
- **Context:** ESPuino was adopted to avoid writing firmware, but v1 needs only: read a tag, play one MP3, pause on lift, remember a position, two buttons. WiFi/web/FTP/OTA turned out to be overkill, and customising ESPuino means learning a large codebase to change small things. Samuel has the hardware and wants a program small enough to understand.
- **Decision:** Write a minimal Arduino/PlatformIO firmware: PN532 over I2C, MP3 from SD via ESP8266Audio, `Preferences` bookmarks, two buttons, no network stack.
- **Consequences:** No web UI, FTP, OTA, modification cards, play modes, NeoPixel or encoder in v1. Stories are added by pulling the SD card. PSRAM/WROVER is no longer required. Maintained by us; the surface area is small.
- **Status:** accepted; supersedes ADR-004 (controls) and ADR-005 (ESPuino) for v1. ADR-001's WROOM board is now sufficient.

## ADR-008 — NFC module: PN532 over I2C
- **Date:** 2026-11-15
- **Context:** ADR-003 left RC522 as the default with PN5180 optional. The module on hand is a PN532.
- **Decision:** Use the PN532 in I2C mode (SDA 21, SCL 22, 3.3 V) with `Adafruit_PN532`.
- **Consequences:** Two signal pins, no contention with the SD SPI bus. The module's DIP switches must be set to I2C. The Adafruit library reports one tag per poll, so stacked-figurine handling is best-effort.
