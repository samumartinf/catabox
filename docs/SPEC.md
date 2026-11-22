# CataBox — Specification

> Source of truth for scope and architecture. Change via PR so both of us see it.
> Status: **v0.3 — minimal custom firmware (ADR-007).**
> Detailed v1 design: [2026-11-15-catabox-v1-firmware-design.md](superpowers/specs/2026-11-15-catabox-v1-firmware-design.md).

## 1. Vision

Catalina places a figurine on the box; the box plays that character's story. Custom Toniebox, made by us — **recipient: Catalina's cousin** (gift for the Christmas break).

**Principles**

1. **Kid-proof first.** No screens, two buttons, survives drops. Figurine on → play, off → pause.
2. **Offline playback.** Audio on SD; no network at playtime or at all in v1.
3. **Small enough to understand.** Our own minimal firmware, not a config-heavy platform.
4. **Content is ours.** Family voices, Spanish and English stories, one MP3 per figurine.

## 2. Platform decision: minimal custom firmware (ADR-007)

The earlier plan adopted **ESPuino** (ADR-005) to avoid writing firmware. We are
reversing that for v1: the scope is small (play/pause/restart plus tag lookup),
the ESPuino codebase is large and config-driven, and we want a program we can
debug line by line.

What we give up: web upload, FTP, OTA, modification cards, built-in play modes.
Stories are added by pulling the microSD card.

What we keep: offline playback, per-figurine bookmarks, and a codebase that fits
in one evening's reading.

**Consequences:** no WiFi requirement, no PSRAM requirement, no WROVER needed. A
plain ESP32 devkit is enough. Archived ESPuino notes remain in
[espuino-plan.md](espuino-plan.md) for reference.

## 3. Architecture

```
┌─────────────┐     ┌────────────────────────────────┐
│ Figurine    │     │ CataBox firmware (ESP32)       │
│ 3D printed  │     │  PN532 NFC (I2C)               │
│ + NTAG213/  │──→  │  microSD (SPI) → MP3           │
│ 215 sticker │     │  MAX98357A I2S DAC + speaker   │
│ + magnet    │     │  2 buttons: pause, restart     │
└─────────────┘     │  NVS: per-tag bookmark         │
                    └────────────────────────────────┘
```

- Place tag → look up `/stories/<UID>.mp3` → play from bookmark.
- Lift tag → pause and save the byte position.
- Replace tag → resume. Restart → 0:00. Pause button toggles.
- Track end clears the bookmark; a missing/broken file is latched until the tag
  is removed.

No network stack is compiled in. The status LED is the only feedback beyond
sound.

## 4. Characters & figurines

- 3D printed bodies ~5 cm, hollow, magnet ring in base matching box recess, NTAG213/215 sticker inside.
- Characters modelled on family: **Samu**, **Ale**, **Catalina** (+ cousin's picks). Animal-face box design (owl/bear/fox — Ale's call).
- Any language audio we record/generate — big win for Spanish-language stories.
- NFC read through PLA + magnets is the main physical risk; test early.

## 5. Bill of materials

See [inventory.md](inventory.md). **~£40–55 total.**

**Have / ordered:** arcade buttons, NFC tags, magnets.

**Verify on hand:**
- [ ] ESP32 devkit (PSRAM not required now)
- [ ] PN532 NFC module (I2C)
- [ ] MAX98357A
- [ ] microSD module + 16 GB card
- [ ] Speaker (Visaton FR10 or similar)
- [ ] 5 V 2 A supply
- [ ] PLA, wiring

**Nice to have later:** NeoPixel ring, rotary encoder, battery/charger.

## 6. Risks / open questions

- **NFC through base + magnets:** detuning can cut range; test with the printed base as soon as possible.
- **PN532 mode:** must be set to I2C via its DIP switches; wiring assumes I2C.
- **MP3 seek:** byte-offset seek resyncs on the next frame; ±2 s resume accuracy.
- **Stacked figurines:** one tag at a time is supported; stacking may confuse the reader.
- **Volume cap for toddler ears:** fixed gain in `config.h`, tune after listening.
- **Battery:** deferred; v1 runs from USB 5 V.

## 7. Milestones (Christmas-break deadline)

1. **Phase 0 — Spec, inventory, orders** ← inventory still open
2. **Phase 1 — Firmware written** ✅ core, buttons, bookmarks, wiring diagram, host tests
3. **Phase 2 — Hardware bring-up:** use `diag_nfc` / `diag_sd` / `diag_audio` to verify each part, then flash the full app and get the first tag→story
4. **Phase 3 — Enclosure & figurines:** case, 2–3 characters with tags/magnets
5. **Phase 4 — Assembly & content:** solder, load stories, parent instructions
6. **Phase 5 — Ship 🎁** (before Christmas break)

---

# Appendix — superseded plans

- **ESPuino (ADR-005):** mature off-the-shelf firmware with web UI/FTP. Rejected for v1 because the feature set is far larger than needed and the codebase is hard to modify. Notes: [espuino-plan.md](espuino-plan.md).
- **Original custom-firmware sketch (v0.1):** modules `nfc` / `player` / `store` / `ui` / `power`, manifest.json per character. The v0.3 implementation is a smaller version of the same idea.
