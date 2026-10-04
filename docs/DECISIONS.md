# Decision Log (ADRs)

Append-only. One entry per decision. Format: context → decision → consequences.

## ADR-001 — Platform: ESP32 (WROOM-32 devkit)
- **Date:** 2026-10-04
- **Context:** Need WiFi-capable MCU (optional future), I2S audio, SD, cheap; Samuel has one.
- **Decision:** ESP32 devkit v1, Arduino/PlatformIO.
- **Consequences:** MP3 decode is CPU-tight but proven (ESP8266Audio / ESP32-audioI2S). 3.3 V logic — level-shift SD module if it's a 5 V one.

## ADR-002 — Playback fully offline from SD; cloud only for content creation
- **Date:** 2026-10-04
- **Decision:** Audio files on microSD; no network at playtime. WiFi kept available for OTA/future content sync.
- **Consequences:** Simpler, more reliable, kid-safe. Content updates via laptop + USB/SD reader.

## ADR-003 — NFC = NTAG sticker in figurine base, read via PN532
- **Date:** 2026-10-04
- **Open:** confirm PN532 vs MFRC522 module on hand; magnets may detune tag — Phase 1 spike.

## ADR-004 — Volume = two physical buttons for MVP
- **Date:** 2026-10-04
- **Decision:** Big tactile buttons ("ears"). Tilt/gesture (MPU) deferred.
