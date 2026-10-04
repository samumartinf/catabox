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

## ADR-003 — RFID: RC522 default, PN5180 optional upgrade
- **Date:** 2026-10-04
- **Open:** confirm which module is on hand; magnets may detune tag — test early.

## ADR-004 — Controls: arcade buttons + rotary encoder for MVP
- **Date:** 2026-10-04
- **Decision:** 3× 30mm arcade buttons (play/prev/next) + KY-040 encoder for volume; NeoPixel ring for feedback. Tilt/gesture deferred.
- **Consequences:** Matches ESPuino's supported GPIO scheme; no custom input code.

## ADR-005 — Firmware: adopt ESPuino instead of custom firmware ⭐
- **Date:** 2026-10-04
- **Context:** Drafted our own firmware spec (nfc/player/store/ui/power modules) before discovering ESPuino. ESPuino is mature (400+★, 1.4k commits) and already implements the whole feature set: web UI, FTP, audiobook position memory, play modes per tag, modification cards (button lock, sleep timer), NeoPixel support, pre-built firmware.
- **Decision:** Use ESPuino. Start with pre-built firmware; compile only when we need custom GPIO changes (`settings-custom.h`).
- **Consequences:** Weeks of firmware work avoided; our effort shifts to enclosure, figurines, and content — the parts that make it *CataBox*. Hardware constraint: ESP32-WROVER with PSRAM is mandatory.
- **Note:** the figurine characters + family voice stories remain fully custom — that's our differentiator.

## ADR-006 — Enclosure: animal-face design (not rocket)
- **Date:** 2026-10-04
- **Decision:** Owl/bear/fox/cat/puppy design (pick in Phase 3). Flat top with NFC recess (~39mm coil) + magnet recesses; speaker grille as mouth/belly; buttons as paws/ears; NeoPixel ring as eyes/collar.
- **Open:** which animal — Ale's call 😄
