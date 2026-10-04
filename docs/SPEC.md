# CataBox — Specification

> Source of truth for scope and architecture. Change via PR so both of us see it.
> Status: **v0.2 — pivoted to ESPuino-based build.**
> Original from-scratch firmware plan kept at the bottom for reference.

## 1. Vision

Catalina places a figurine on the box; the box plays that character's story. Custom Toniebox, made by us — **recipient: Catalina's cousin** (gift for the Christmas break).

**Principles**

1. **Kid-proof first.** No screens, few buttons, survives drops. Figurine on → play, off → pause.
2. **Parents manage content over WiFi** — no opening the box, no SD-card removal at playtime.
3. **Offline playback.** Audio on SD; WiFi used only for management (web UI, FTP).
4. **Don't rebuild what exists.** Use the mature ESPuino firmware instead of writing our own.

## 2. Platform decision: ESPuino (ADR-005)

We are adopting **ESPuino** (biologist79/ESPuino, mature OSS) instead of custom firmware:

| We get for free | Why it matters |
|---|---|
| Web interface (phone browser) | Assign NFC tags to audio, upload MP3s over WiFi |
| FTP server | Bulk-load stories like a network drive |
| Audiobook position memory | Resume exactly where the story stopped |
| Play modes per tag | Audiobook mode is exactly our story use case |
| Modification cards | Lock buttons (toddler!), sleep timer, night mode |
| NeoPixel ring support | Track progress / volume / battery as kid-friendly light show |
| Pre-built firmware | No compile needed to get started; compile only for customisation |

**Hardware requirement:** the ESP32 board **must have PSRAM → ESP32-WROVER** (not plain WROOM). Check what we have on hand before ordering.

RC522 is the default RFID module; PN5180 is the optional upgrade (better range, LPCD low-power wake, can read original Tonies).

## 3. Architecture

```
┌─────────────┐     ┌───────────────────────────────┐
│ Figurine    │     │ CataBox (ESPuino)             │
│ 3D printed  │     │  ESP32-WROVER (PSRAM)         │
│ + NTAG213/  │──→  │  RC522 RFID (SPI)             │
│ 215 sticker │     │  microSD (SPI)                │
│ + magnet    │     │  MAX98357A I2S DAC + speaker  │
└─────────────┘     │  3× arcade buttons + KY-040   │
                    │  NeoPixel ring                │
                    │  USB/18650 + LiFePO4 charger  │
                    └───────────────────────────────┘
                            ▲ WiFi (management only)
                    Parents: web UI upload / FTP / tag assignment
```

### Wiring (ESPuino `settings-custom.h`)

Full pin map is in [espuino-plan.md](espuino-plan.md) (I2S, RC522 SPI, SD SPI, buttons, encoder, NeoPixel). Copy it into `src/settings-custom.h` when we compile; if using pre-built firmware pick the closest supported board config.

## 4. Characters & figurines

- 3D printed bodies ~5 cm, hollow, magnet ring in base matching box recess, NTAG213/215 sticker inside.
- Characters modelled on family: **Samu**, **Ale**, **Catalina** (+ cousin's picks). Animal-face box design (owl/bear/fox — decide in Phase 3); top surface flat with NFC recess + magnets.
- Any language audio we record/generate — big win for Spanish-language stories.

## 5. Bill of materials

See [espuino-plan.md](espuino-plan.md) for the full list + UK links. **~£40–55 total.**

**Status: buttons and some other items already ordered.** Inventory before ordering anything else — update [inventory.md](inventory.md) as items arrive.

### To verify on hand / still to order
- [ ] ESP32-WROVER **with PSRAM** (check existing boards; order WROVER if not)
- [ ] RC522 (have? PN5180 upgrade optional)
- [ ] MAX98357A
- [ ] microSD module + 16GB card
- [ ] Speaker (Visaton FR10 or similar)
- [ ] NeoPixel ring
- [ ] Rotary encoder KY-040
- [ ] NFC tags NTAG213/215 (have some)
- [ ] Magnets, PLA, wiring (have)

## 6. Risks / open questions

- **PSRAM:** plain WROOM-32 boards won't run ESPuino well — inventory check is the first task.
- **NFC through base + magnets:** detuning risk; test early.
- **SD on SPI vs SDMMC:** ESPuino prefers SD_MMC (1-bit) where possible — fewer pins, faster.
- **Volume cap for toddler ears:** configure max volume in web UI config.
- **Battery choice:** LiFePO4 recommended by ESPuino community for safety.
- **Story generation:** MVP = recorded family voices / existing MP3s. TTS/generated stories = stretch (Phase 5).

## 7. Milestones (Christmas-break deadline)

1. **Phase 0 — Spec, inventory, orders** ← current
2. **Phase 1 — ESPuino bring-up:** flash pre-built firmware, breadboard, verify audio + NFC + web UI
3. **Phase 2 — Wire-up & config:** real pin map, buttons/encoder/NeoPixel, tag→story assignment, modification cards
4. **Phase 3 — Enclosure design & print:** animal-face case, figurines (2–3)
5. **Phase 4 — Assembly & content:** solder, load stories, parent instruction sheet
6. **Phase 5 — Ship 🎁** (before Christmas break)

---

# Appendix — original from-scratch plan (v0.1, superseded)

<details>
<summary>Custom firmware approach (kept for reference)</summary>

Original plan: own ESP32 firmware with modules `nfc` / `player` / `store` / `ui` / `power`; MP3-or-WAV decode decision; manifest.json per character. This is exactly the feature set ESPuino already ships — see ADR-005.

</details>
