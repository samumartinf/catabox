# Roadmap

Living doc — update as we go. **Deadline: ready before the Christmas break.**

## Phase 0 — Spec, inventory, orders (current)
- [x] Repo created; ESPuino plan folded into spec (v0.2)
- [x] Review spec with Ale → decisions logged (ADR-005, ADR-006)
- [ ] **Inventory electronics on hand — especially: does our ESP32 have PSRAM?** (WROVER required)
- [ ] Check buttons/NFC tags/magnets already ordered
- [ ] Order remaining parts (see inventory.md) — mind shipping times
- [ ] Pick animal design for the box (Ale's call)

## Phase 1 — ESPuino bring-up (breadboard)
- [ ] Flash pre-built ESPuino firmware to ESP32-WROVER
- [ ] First boot: join WiFi AP, verify web UI reachable
- [ ] Wire MAX98357A + speaker → play an MP3 from SD
- [ ] Wire RC522 → tap a tag, see it appear in web UI
- [ ] Assign a tag to a folder → tap = play, lift = pause
- [ ] Test audiobook mode position memory

## Phase 2 — Wire-up & config
- [ ] Final pin map in `settings-custom.h` (or pick matching pre-built config)
- [ ] 3× arcade buttons + KY-040 encoder working
- [ ] NeoPixel ring: progress/volume/battery states
- [ ] Set max volume cap (toddler ears)
- [ ] Modification cards: button-lock card + sleep-timer card programmed
- [ ] FTP upload tested from Ale's phone

## Phase 3 — Enclosure & figurines
- [ ] CAD: animal-face box (Fusion 360) — flat top, NFC recess 39mm, magnet recesses, speaker grille, button holes, USB access
- [ ] Test-fit print, iterate
- [ ] Design 2–3 figurines (family characters) with NFC stickers + magnets in base
- [ ] Final print + post-process (sand)

## Phase 4 — Assembly & content
- [ ] Solder on perfboard, mount components
- [ ] Load stories/songs (Spanish + English), assign tags via web UI
- [ ] Write one-page parent instructions (web UI access, FTP creds, lock card)
- [ ] Kid-proofing pass: rounded, no small parts, drop test

## Phase 5 — Ship 🎁
- [ ] Wrap and deliver before Christmas
