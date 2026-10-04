# Getting Started — Samuel's Phase 1 Plan

First hardware project in 8+ years? Welcome back. The good news: ESPuino means
you're mostly *configuring* firmware, not writing it. The even better news:
every step below has a cheap win you can feel the same evening.

## Mindset rules

1. **One subsystem at a time, always end with a demo.** Blink, then beep, then tap. Never wire everything at once.
2. **Power off before rewiring.** USB unplugged, every time.
3. **Keep a lab notebook** in this repo (`docs/lab-log.md`). Date, what you tried, what happened. In 3 weeks you'll thank yourself.
4. **Photos of every breadboard state** → `docs/photos/`. Wiring errors are 90% of beginner pain; photos make them findable.
5. Git-commit after every working milestone. If it works, it's committed.

## Your first session (~90 min, tonight, zero soldering)

**Goal: blink + serial.** Proof your board, cable, and toolchain work.

1. If you haven't got VS Code + PlatformIO: install, create new project, framework Arduino, board `Espressif ESP32 Dev Module`.
2. Blink sketch (`File > Examples > 01.Basics > Blink`, LED on GPIO2 on most devkits).
3. Open Serial Monitor at 115200 — see `Hello`.
4. **Commit. Log it.**

✅ Cheap win #1.

## Check: do you have PSRAM? (decides everything)

```cpp
// sketch → run, read Serial Monitor
void setup() {
  Serial.begin(115200);
  Serial.printf("PSRAM size: %d bytes\n", ESP.getPsramSize());
  Serial.printf("Free PSRAM: %d\n", ESP.getFreePsram());
}
```

- If `0` → you have a WROOM. Order an **ESP32-WROVER** today (shipping is your critical path before Christmas). You can still do most of Phase 1 on the WROOM with plain Arduino code; ESPuino just needs PSRAM.
- If `4194240` or similar → you're on WROVER, proceed straight to ESPuino.

## Session 2: NFC — read your first tag (~60 min)

**Goal: print a figurine tag's UID over serial.**

1. Library: `Adafruit_PN532` (PN532, I2C/HSU) or `MFRC522` (RC522, SPI) — whichever module you actually have. Photo of the module → we pick the right library.
2. Wire per the pin map in `docs/espuino-plan.md`, hold tag near antenna.
3. Sketch prints the UID in hex.

✅ Cheap win #2: *"this piece of plastic knows who it is."*

Now tap each figurine tag once, write the UIDs in `characters/tags.md` — that table later becomes the tag→story assignment (and if you use ESPuino's web UI, it learns tags the same way).

## Session 3: Make a sound (~90 min)

**Goal: MP3 from SD → speaker, using an Arduino sketch (not ESPuino yet).**

1. Wire MAX98357A per pin map (3 wires + power). Speaker on output.
2. SD module → SPI. Format a 16GB card FAT32, drop one MP3 in.
3. Use `ESP32-audioI2S` (schreibfaul1) or `ESP8266Audio` demo sketch.
4. Hear sound → **✅ Cheap win #3: the box speaks.**

At this point you have proven every subsystem ESPuino needs, independently, with sketches you can debug line by line.

## Session 4: Flash ESPuino (the big merge)

1. Flash pre-built firmware for your board from `biologist79/ESPuino-Firmware` (or build with PlatformIO once you want custom GPIOs).
2. First boot → join `ESPuino` WiFi AP → captive portal → home WiFi.
3. Open web UI, tap a tag, assign it to a folder of MP3s.
4. **✅ Cheap win #4: it's a Toniebox.** Everything after this is polish.

## Order *today* (critical path = shipping, not skill)

- MAX98357A (if not on hand)
- microSD module + card
- Speaker (Visaton FR10)
- RC522/PN532 (whichever you lack)
- ESP32-WROVER if the PSRAM check fails

~£35 of parts. Everything else (buttons, tags, magnets, filament) you have or Ale's prints can wait.

## Division of labour

| Track | Owner | Starts |
|---|---|---|
| Firmware bring-up, wiring, ESPuino config | Samuel | now (sessions 1–4 above) |
| Figurine + enclosure CAD | Ale | now (can design before electronics work) |
| Character stories (recordings, scripts) | Both | anytime, zero hardware needed |

**Ale's first deliverables** (independent of your sessions, no ordering risk):
1. Pick the animal (owl/bear/fox/cat/puppy) → sketch on paper.
2. Model one figurine, print, embed an NFC tag + magnet — even before the box works, she can test tag-read-through-print on your Session-2 breadboard.
3. Fusion 360 free tier is fine; [Biobox 3D](https://forum.espuino.de/t/biobox-3d/3130) case is a proven base to remix instead of starting from scratch.

## Failure playbook

- Sketch won't upload → wrong board selected / bad cable (charge-only micro-USB is the #1 trap) / GPIO0 held low.
- NFC never reads → check 3.3V (not 5V), I2C vs SPI mode jumpers on PN532, or wiring swap on RC522.
- Audio crackles → SD too slow (use good Class 10), or shared SPI pin conflicts.
- 3+ failed fixes on the same problem → write up in lab-log.md, take a photo, walk away, come back tomorrow. (Or ping me.)
