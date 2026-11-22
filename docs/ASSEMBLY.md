# Assembly & soldering guide

For moving CataBox from “held together with fingers” to a build that survives a
toddler. The electronics do not need a custom PCB; a perfboard with headers and
terminal blocks is enough.

## Tools

- Temperature-controlled soldering iron (330–350 °C leaded, 350–370 °C lead-free)
- 0.8 mm rosin-core solder, flux, brass wool / damp sponge
- Flush cutters, wire strippers, tweezers, helping hands or a small vice
- Multimeter with continuity mode
- 2.54 mm male and female headers, perfboard
- 22 AWG wire for power, 26–30 AWG for signals, heat-shrink
- 470 µF electrolytic capacitor, 5 V supply, USB cable

## Rules

1. **Do not solder the ESP32 directly.** Solder female headers to the board and
   plug the devkit in. It stays removable and unburned.
2. **Power off and unplug USB before every wiring change.**
3. **Test continuity before power.** A short between 5 V and GND is the fastest
   way to end the project.
4. **One subsystem at a time.** Power rails → NFC → SD → audio → buttons.
5. **Keep I2S wires short** (BCLK, LRC, DIN), and keep a solid ground return next
   to them. Hand-held jumper wires will produce noise no matter how good the amp.

## Order of work

### 1. Solder headers to the modules

- MAX98357A: 7-pin male header (LRC, BCLK, DIN, GND, VIN, GAIN, SD). If the
  speaker terminal block is not attached, solder that too.
- PN532: male header for VCC, GND, SDA, SCL. Set the DIP switches to I2C.
- microSD module: male header for VCC, GND, CS, SCK, MISO, MOSI.
- ESP32: female headers on the perfboard; the devkit plugs in.

If a module already has a soldered header, use female-to-female dupont wires for
the prototype and skip this step.

### 2. Power rails

- 5 V rail: ESP32 `5V`/`VIN`, SD module VCC if it has a regulator, MAX98357A VIN.
- 3.3 V rail: PN532 VCC, SD module VCC only if it is a bare 3.3 V board.
- GND rail: ESP32, PN532, SD module, amp, both buttons.
- 470 µF across 5 V/GND near the amp; 100 nF near the SD module if you have one.
- Use red for 5 V, orange for 3.3 V, black for GND, and keep the rails separate.

### 3. Buttons

- Pause → GPIO 32, restart → GPIO 33, other terminal → GND.
- 30 mm arcade buttons use 4.8 mm spade terminals; crimp or solder, then
  heat-shrink. Internal pull-ups are enabled in firmware, so no resistors.

### 4. NFC

- PN532 SDA → GPIO 21, SCL → GPIO 22, VCC → 3.3 V, GND → GND.
- Keep the antenna away from metal, the SD card, and the speaker magnet.
- DIP switches: `SEL0 ON`, `SEL1 OFF` (I2C).

### 5. microSD

- CS → GPIO 5, SCK → GPIO 18, MISO → GPIO 19, MOSI → GPIO 23.
- VCC → 5 V if the module has a regulator, otherwise 3.3 V.
- Keep the card slot accessible from the enclosure.

### 6. Audio

- MAX98357A LRC → GPIO 26, BCLK → GPIO 27, DIN → GPIO 25.
- VIN → 5 V, GND → common, GAIN open (9 dB) or 100 kΩ to VIN (3 dB).
- SD → VIN to enable (left channel) or leave open if the board has its own
  pull-up. It is the shutdown/mode pin, not the SD card.
- Speaker red/black → the amp’s output screw terminal. The output is
  bridge-tied: never connect either speaker wire to GND.
- Keep the I2S wires short and twisted with a ground return if possible.

### 7. Test sequence

```bash
pio run -e diag_i2c -t upload && pio device monitor -b 115200
pio run -e diag_nfc -t upload
pio run -e diag_sd -t upload
pio run -e diag_audio_quiet -t upload
pio run -e esp32dev -t upload
```

Expected: `0x24` on I2C, `PN532 ready` + UIDs, `SD: mounted`, a clean 30-second
tone at low volume, then the full app.

## Soldering technique

- Tin the tip; wipe on brass wool.
- Heat the **pad and the lead together** for 1–2 seconds, then feed solder into
  the joint (not onto the iron).
- Remove the solder, then the iron; hold still while it cools.
- A good joint is shiny and concave. A dull ball is a cold joint: add flux and
  reheat.
- Use a little flux for header rows; clean with IPA afterwards.
- Inspect with a magnifier for bridges between adjacent pins.
- Check every connection with continuity before applying power.

## Mechanical notes

- Hot-glue or zip-tie the SD and amp wires so the card slot and header are not
  taking the strain.
- Keep the PN532 antenna clear of the speaker magnet and any metal bracket.
- Leave the USB port accessible; the box is powered from USB 5 V in v1.
- Do not hot-glue over the SD card slot or the PN532 antenna.

## When it still sounds bad

- Hand-held wires, loose headers, and a missing ground will always sound bad.
  A 1.4 MHz I2S clock needs solid connections, not faith.
- If the amp gets hot or the ESP32 resets, stop: check for a 5 V/GND short,
  add the 470 µF capacitor, and use a better USB supply.
- Test the speaker independently with `diag_tone` and a 220–330 Ω resistor before
  blaming the amp.
