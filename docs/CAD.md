# Enclosure & 3D-print plan (Bambu Lab)

Status: draft. Dimensions below are placeholders until the electronics are measured.

Printer: **Bambu Lab A1** — 256 × 256 × 256 mm build volume. Keep every part
under ~250 mm per axis so it fits with a brim; split the shell if needed.

## Modular design

Print the box as parts, not one monolith:

1. **Electronics tray** — base with mounting posts for the ESP32, PN532, SD module,
   amp, and cable routes. USB and SD cutouts.
2. **Top shell / animal face** — separate cosmetic top with the NFC platform,
   magnet pockets, speaker grille, and button holes. Removable for service.
3. **Figurines** — character bodies with pockets for an NTAG213/215 sticker and a
   10×3 mm magnet in the base.
4. **Small parts** — button bezels/retainers, speaker clamp, cable clips, NFC
   spacer, a soldering jig, and a fit-test coupon.

## Measure before modelling

| Part | Needed dimensions |
|---|---|
| ESP32 devkit | length, width, tallest component, mounting hole spacing |
| PN532 | board size, coil/antenna area, connector height |
| SD module | board size, card protrusion, connector height |
| MAX98357A | board size, header/screw-terminal height |
| Speaker | outer frame, depth, mounting-hole spacing (centre-to-centre), screw size |
| Arcade buttons | hole diameter, panel thickness range, nut/thread depth |
| Magnets | 10 mm diameter × 3 mm (confirm) |
| NFC sticker | antenna diameter (~39 mm clear area) |
| USB cable | connector overmold size for the cutout |

Add 2–3 mm clearance around each board and 4–6 mm around connectors. Use
calipers, not a ruler, for the speaker mounting holes and button nut depth.

## Bambu print settings (starting point)

- 0.4 mm nozzle, 0.20 mm layers, 3 walls, 4 top/bottom layers, 15 % gyroid.
- **PETG** for the shell if it will be dropped; **PLA/PLA+** for the face and
  figurines (better detail and colour). PETG: ~240–250 °C, bed 70–80 °C.
- Textured PEI plate; glue stick for PETG if needed.
- Brim for tall/thin parts; supports only under the animal face overhangs.
- Ironing on visible top surfaces; fuzzy skin on fur, **not** over the NFC area.
- Export a **3MF** from Bambu Studio when using AMS/colour changes.
- Print the tray open-side-up and the top face outer-side-down for a smooth face.

## Fit rules

- 0.25–0.30 mm clearance for printed sliding fits; 0.15 mm for press fits.
- M3 heat-set inserts: 4.2 mm hole, 5.5–6.0 mm deep; test on a coupon.
- Magnet pockets: 10.2 mm diameter, 3.2 mm deep; glue, check polarity before
  setting.
- Button holes: 30.2–30.5 mm, panel 2–4 mm; check the nut/thread depth.
- Screw bosses: 6 mm OD, 3.4 mm pilot for M3 self-tappers, or inserts above.
- Chamfer/fillet all kid-facing edges; avoid parts smaller than a choke gauge.
- Keep metal screws, magnets, and the SD card away from the PN532 antenna.

## NFC and speaker

- Put the tag under **1.6–2.0 mm** of PLA over the PN532 coil; test read range
  before committing to the final top.
- Keep a **39 mm clear area**; no screws, magnets, or foil near it.
- Magnet pockets around the perimeter, at least 15 mm from the tag centre.
- Speaker: print a recess with a foam gasket and a 3–4 mm grille; give the amp
  its own ventilated compartment. Keep I2S wires short.

## First print: fit coupon

Before the full box, print a 60 × 60 × 4 mm coupon containing:

- one 10.2 mm magnet pocket,
- one 4.2 mm heat-set insert hole,
- one 30.4 mm button hole,
- a 1.8 mm test plate over an NFC tag.

Test the fits and the tag range, then adjust the parameters and print the tray.

## CAD and AI workflow

- **Fusion 360 (free personal)** for the parametric tray, top, and mounts.
- **Blender** for the animal face/figurines, or Fusion sculpt.
- **OpenSCAD / CadQuery + an LLM** for a fully code-driven, parametric box; this
  is the easiest workflow to iterate with an agent.
- **Meshy / Tripo / Rodin / MakerWorld AI** for concept meshes and figurine
  ideas; expect cleanup and no dimensional accuracy.
- **Zoo Text-to-CAD / Backflip** are promising for prompt/scan-to-CAD, but still
  verify every dimension yourself.
- Bambu Studio is a slicer, not a CAD modeller.

## Assembly order

1. Print the fit coupon; tune clearances.
2. Print the tray and top in draft; test-fit the electronics, buttons, speaker.
3. Test NFC range through the top; adjust thickness/magnet placement.
4. Final print in PETG/PLA+; install heat-set inserts.
5. Mount the perfboard, speaker, buttons, and amp; strain-relieve wires.
6. Close the box; test all controls; then print the figurines.
