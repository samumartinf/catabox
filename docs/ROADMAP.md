# Roadmap

Living doc — update as we go.

## Phase 0 — Spec & design (current)
- [x] Repo created, README + spec drafted
- [ ] Review spec together (Samuel + Ale)
- [ ] Inventory what electronics we actually have (photos → `docs/inventory.md`)
- [ ] Confirm BOM, order missing parts
- [ ] Decide story source for MVP (recorded voices vs TTS)

## Phase 1 — Hardware spike
- [ ] Breadboard: ESP32 + PN532, read a figurine tag UID over serial
- [ ] SD card + MAX98357A: play an MP3
- [ ] Combined: tap figurine → play file; remove → pause
- [ ] NFC read-through-base test (tag + magnet + print thickness)
- [ ] Decision log entry: audio format (MP3 vs WAV)

## Phase 2 — MVP firmware
- [ ] Manifest format + UID→story mapping (JSON on SD)
- [ ] Pause/resume, per-character resume position (NVS)
- [ ] Volume buttons (cap max output)
- [ ] Idle deep sleep + wake
- [ ] Content sync script (laptop → SD structure)

## Phase 3 — Characters & content
- [ ] Design + print first 3 figurines (Catalina, Samu, Ale)
- [ ] Character manifest + stories for each
- [ ] Story generation/TTS pipeline (separate spec if needed)

## Phase 4 — Enclosure & power
- [ ] Box CAD (recess with magnet + NFC coil alignment, "ear" buttons)
- [ ] Battery + USB-C charging integration
- [ ] Kid-proofing pass (rounded, screwless access, drop test)

## Phase 5 — Ship 🎁
- [ ] Wrap, deliver, watch Catalina's face
