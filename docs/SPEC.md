# CataBox — Specification

> Source of truth for scope and architecture. Change via PR so both of us see it.
> Status: **DRAFT v0.1** — awaiting sign-off from Samuel & Ale.

## 1. Vision

Catalina places a figurine (a character resembling a family member) on the box; the box plays a story for that character. Custom Toniebox, made by us.

**Principles**

1. **Toddler-proof first.** No screens, few buttons, survive drops. Kid-operable: figurine on → story starts, figurine off → stops.
2. **Parents manage content** from a laptop/phone; the box is a dumb-but-reliable player.
3. **Local, not cloud-dependent at playtime.** Audio stored on the box (SD card). Generation can use the cloud, playback never needs it.
4. **Built, not bought.** 3D printed enclosure, off-the-shelf electronics.

## 2. User experience (MVP)

1. Catalina picks a figurine and puts it on top of the box (magnet-seated).
2. NFC reader identifies the figurine → box starts that character's story.
3. Lifting the figurine off → story pauses; replacing it resumes or switches.
4. Two big buttons: **volume −** and **volume +** (side "ears", like the real Toniebox). Tilt/knock gestures are a stretch goal, not MVP.
5. Battery powered, chargeable over USB-C.

**Explicitly out of MVP scope:** creative/tap-to-record features, WiFi streaming, companion app, multi-story-per-character,cloud playback.

## 3. Architecture

```
┌─────────────┐     ┌──────────────────────────┐
│ Figurine    │     │ CataBox                  │
│ 3D printed  │     │  ESP32 (devkit WROOM-32) │
│ + NFC tag   │──→  │  PN532 NFC (I2C/HSU)     │
│ + magnet    │     │  SD card (SPI)           │
└─────────────┘     │  I2S DAC (MAX98357A)     │
                    │  + speaker (3–5 W)       │
                    │  2× buttons (GPIO)       │
                    │  LiPo + TP4056 charger   │
                    └──────────────────────────┘
```

- **ESP32** runs firmware: poll NFC (a few Hz), look up UID → story, stream audio from SD to I2S DAC.
- **Content model:** each figurine's NFC UID maps to a story manifest (`characters/<name>/manifest.json`) listing tracks, order, resume position.
- **Content pipeline (laptop):** scripts to (a) register a new figurine UID, (b) convert/place audio files on SD in the right structure, (c) later: text → story generation pipeline (TTS or recorded family voices).

### Firmware modules

| Module | Responsibility |
|---|---|
| `nfc` | Read UID from PN532, debounce, on/off events |
| `player` | Audio decoding + I2S output, pause/resume per character |
| `store` | Manifest lookup by UID, resume positions persisted (NVS) |
| `ui` | Volume buttons, status LED (ear-like buttons phase 1) |
| `power` | Battery/charging state, deep sleep when idle |

### Audio format

MP3 (ID3-stripped), 44.1 kHz stereo or mono, ~128 kbps. ESP32-audioI2S or ESP8266Audio library decode; verify decode load on ESP32 in Phase 1 spike. WAV fallback if MP3 decoding is flaky (SD space is fine).

## 4. Characters & figurines

- 3D printed body ~5 cm tall, hollow, magnet ring in base matching box recess; NFC tag (NTAG213/215 sticker) inside the base.
- Each character modeled on a real person: e.g. **Samu**, **Ale**, **Catalina** (+ any requested characters).
- Character definition lives in `characters/<name>/`: model/STL or reference art, manifest.json, voice notes.

## 5. Bill of materials (initial — confirm before buying)

| Item | Qty | Est. |
|---|---|---|
| ESP32 devkit (WROOM-32) | 1 | have |
| PN532 NFC module | 1 | have (verify) |
| MAX98357A I2S DAC | 1 | ~£4 |
| Full-range speaker 3–5 W, 4 Ω | 1 | ~£5 |
| microSD card + SPI module | 1 | ~£6 |
| TP4056 + protection, LiPo 2000 mAh | 1 | ~£8 |
| Tactile/arcade buttons ×2 | 2 | ~£3 |
| Magnets + NFC tags | — | have |
| Filament, fasteners | — | have |

## 6. Risks / open questions

- **NFC through the printed base + magnet:** test tag read distance with magnets nearby (Phase 1 spike).
- **MP3 decode + I2S on ESP32 simultaneously:** spike early; fall back to WAV.
- **Power:** deep sleep required; NFC polling wakes it? Decide: sleep between polls vs wake-on-tap.
- **Speaker loudness/safety:** cap max volume in firmware (kid ears).
- **Story generation:** MVP = any MP3 we put on SD. Story writing/TTS pipeline = Phase 3.

## 7. Milestones

1. **Phase 0 — Spec & design** ← we are here
2. **Phase 1 — Hardware spike:** NFC read + play an MP3 from SD through speaker, on a breadboard.
3. **Phase 2 — MVP firmware:** UID→story mapping, pause/resume, volume buttons, manifest structure.
4. **Phase 3 — Characters:** 3D figurines printed, story content pipeline, 3+ characters loaded.
5. **Phase 4 — Enclosure & power:** printed box, battery, USB-C charging, kid-proofing.
6. **Phase 5 — Ship to Catalina.** 🎁
