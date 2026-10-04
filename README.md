# CataBox

A DIY Toniebox for **Catalina** — ESP32-based audio box with NFC figurines. Each figurine is a character (modeled on family members 🎨) that plays its own story when placed on the box.

Built with: ESP32, PN532/MFRC522 NFC reader, I2S DAC + speaker, 3D-printed enclosure, NFC-tag figurines with magnets.

## Status

🚧 Phase 0 — spec & design (no hardware bring-up yet). See [docs/SPEC.md](docs/SPEC.md).

## Repo layout (planned)

```
docs/        specs, decisions, roadmap
firmware/    ESP32 code (PlatformIO / Arduino)
characters/  figurine character definitions + NFC UID mapping
audio/       story content, generation pipeline
cad/         3D-printable enclosure + figurine models
```

## Team

- **samumartinf** — Samuel
- **alemonescolom99** — Ale

## Quick links

- [Spec](docs/SPEC.md) — the single source of truth for scope & architecture
- [Roadmap](docs/ROADMAP.md) — phases and milestones
- [Decisions](docs/DECISIONS.md) — decision log (ADRs)
