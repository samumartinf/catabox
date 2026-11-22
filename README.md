# CataBox

A DIY Toniebox for **Catalina** — ESP32-based audio box with NFC figurines. Each figurine is a character (modeled on family members 🎨) that plays its own story when placed on the box.

Built with: ESP32, PN532 NFC reader, MAX98357A I2S DAC + speaker, 3D-printed enclosure, NFC-tag figurines with magnets.

## Status

🚧 Phase 1 — **firmware written and host-tested**, hardware bring-up next. See [docs/ROADMAP.md](docs/ROADMAP.md).

v1 is deliberately small: place a figurine → its story plays; lift it → pause and
remember the spot; two buttons (pause, restart). No WiFi, no app. Stories are MP3s
on the SD card.

- [Firmware README](firmware/README.md) — build, flash, SD layout, tag workflow
- [Wiring diagram](docs/wiring/catabox-wiring.svg)
- [Assembly guide](docs/ASSEMBLY.md) — soldering and perfboard plan
- [v1 design](docs/superpowers/specs/2026-11-15-catabox-v1-firmware-design.md)

## Repo layout

```
docs/        specs, decisions, roadmap, wiring
firmware/    ESP32 code (PlatformIO / Arduino) + host tests
characters/  figurine character definitions + NFC UID mapping
audio/       story content, generation pipeline (planned)
cad/         3D-printable enclosure + figurine models (planned)
```

## Team

- **samumartinf** — Samuel
- **alemonescolom99** — Ale

## Quick links

- [Spec](docs/SPEC.md) — scope & architecture (v0.3)
- [Roadmap](docs/ROADMAP.md) — phases and milestones
- [Decisions](docs/DECISIONS.md) — decision log (ADRs)
- [Inventory](docs/inventory.md) — what we own vs need
