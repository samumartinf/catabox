# Roadmap

Living doc — update as we go. **Deadline: ready before the Christmas break.**

## Phase 0 — Spec, inventory, orders (current)
- [x] Repo created; v1 custom-firmware design approved (ADR-007)
- [x] v1 firmware written + host-tested
- [ ] Inventory electronics on hand (PN532, MAX98357A, SD module, speaker, ESP32)
- [ ] Order anything missing (see inventory.md) — mind shipping times
- [ ] Pick animal design for the box (Ale's call)

## Phase 1 — Firmware ✅
- [x] Core state machine: tag → play, lift → pause/save, return → resume
- [x] Pause button (sticky) and restart button (tag required)
- [x] Per-figurine bookmarks in NVS, cleared at story end
- [x] 22 host tests with `pio test -e native`
- [x] PN532 (I2C), microSD/MP3, MAX98357A adapters
- [x] Wiring diagram and firmware README
- [ ] Flash and verify on real hardware → Phase 2

## Phase 2 — Hardware bring-up (breadboard)
- [ ] Flash `pio run -e esp32dev -t upload`, confirm serial boot
- [ ] SD mounts; `/stories` visible
- [ ] Place tag, read UID in serial
- [ ] First MP3 plays through speaker
- [ ] Lift pauses; replace resumes from the same spot
- [ ] Pause and restart buttons behave
- [ ] Power cycle resumes from the bookmark
- [ ] Tag reads through a printed base + magnet (range test)

## Phase 3 — Enclosure & figurines
- [ ] CAD: animal-face box — flat top, NFC recess 39 mm, magnet recesses,
      speaker grille, button holes, USB access
- [ ] Test-fit print, iterate
- [ ] Design 2–3 figurines (family characters) with NFC stickers + magnets
- [ ] Final print + post-process (sand)

## Phase 4 — Assembly & content
- [ ] Solder on perfboard, mount components
- [ ] Load stories/songs (Spanish + English), one MP3 per tag
- [ ] Write one-page parent instructions (power, buttons, adding stories)
- [ ] Kid-proofing pass: rounded, no small parts, drop test

## Phase 5 — Ship 🎁
- [ ] Wrap and deliver before Christmas
