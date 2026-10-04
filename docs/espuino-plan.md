# 🚀 DIY Toniebox Project — Build from Scratch (ESP32 Edition)

**Last updated:** 8 July 2026  
**Project:** Build a custom Toniebox for the nephew (Catalina's cousin!)  
**Approach:** **ESPuino** — ESP32-based with WiFi web interface  

---

## 🧠 Project Overview — Why ESP32?

The **ESP32 + ESPuino** approach is the sweet spot: cheaper than Raspberry Pi but adds WiFi for a web interface. His parents can manage audio wirelessly — no need to pull the SD card.

| Approach | Brain | Cost | Skill Level | Key Feature |
|---|---|---|---|---|
| **1. TonUINO** | Arduino Nano | ~£30-40 | Beginner+ | Offline only, no WiFi |
| **2. ESPuino ⭐** | **ESP32-WROVER** | **~£35-50** | **Intermediate** | **Web interface + FTP + WiFi + BT** |
| 3. Phoniebox | Raspberry Pi Zero W | ~£60-80 | Intermediate | Full streaming, slower boot |

**Critical requirement:** ESP32 MUST have PSRAM — use **ESP32-WROVER** (not basic ESP32).

---

## ✅ ESPuino — What Makes It Special

**ESPuino** is a mature open-source project (409★, 1,424 commits) that turns an ESP32 into a full Toniebox with:

### 🔹 WiFi Web Interface
Parents open a browser on their phone and can:
- Assign NFC tags to audio files (drag & drop)
- Upload new MP3 files wirelessly
- See what's playing, control volume, lock buttons
- No app required — works on any device with a browser

### 🔹 FTP Server
Drop audio files over WiFi like a network drive. No need to open the box.

### 🔹 Audiobook Mode with Position Memory
Remembers where the story stopped — even if the tag is removed and put back later.

### 🔹 Bluetooth
Can stream audio **to** Bluetooth headphones, or stream **from** a phone to the box.

### 🔹 Neopixel LED Ring
Shows track progress, volume level, battery status, error states — magical for kids.

### 🔹 MQTT / Smart Home
Can integrate with Home Assistant if they want.

### 🔹 Pre-Built Firmware Available
No need to compile code — ready-to-use firmware is downloadable.

---

## 📦 Complete Parts List

| Part | Description | Approx. Cost | Notes |
|---|---|---|---|
| **ESP32-WROVER Dev Board** | ESP32 with PSRAM (mandatory!) | £8-12 | MUST have PSRAM — look for ESP32-WROVER or ESP32-WROOM with PSRAM |
| **MAX98357A I2S DAC** | I2S audio amplifier module | £4-6 | Better quality than DFPlayer Mini |
| **RC522 RFID Module** | NFC/RFID card reader (13.56MHz) | £2-3 | Cheapest option, works well |
| **PN5180 RFID Module** *(optional upgrade)* | Better range, can read original Tonies | £8-10 | More sensitive, can use LPCD wake |
| **MicroSD Card Module** | SPI or SDMMC card reader | £3-5 | For audio storage |
| **MicroSD Card** | 8-32GB for audio | £5-8 | Class 10 |
| **Speaker** | 4-8 Ohm, 3W (Visaton FR 10 or similar) | £8-12 | ~10cm diameter |
| **Arcade Buttons × 3** | 30mm buttons (play/pause, next, prev) | £5-8 | |
| **Rotary Encoder** | KY-040 for volume control | £2-3 | Much nicer than buttons for volume |
| **NeoPixel Ring** | 12-16 LED ring (recommended!) | £5-8 | Shows progress, volume, battery |
| **Power Supply** | 5V 2A USB or 18650 battery + charger | £5-10 | LiFePO4 batteries recommended for safety |
| **NFC Tags** | NTAG213/215 stickers (pack of 10) | £3-5 | To make custom "Tonies" |
| **PLA Filament** | ~200g, 2 colours | £5-8 | |
| **Neodymium Magnets** | 10×3mm | £3 | To hold Tonies on top |
| **Wiring & Connectors** | Jumper wires, solder, perfboard | £5 | |

**Total: ~£40-55** (vs £80+ for real Toniebox + creative Tonies)

### 🔌 Wiring Diagram — ESP32 Dev Board

**ESP32 → MAX98357A (I2S DAC + Amp):**
```
ESP32-WROVER        MAX98357A
GPIO25 (DOUT)  →    DIN
GPIO27 (BCLK)  →    BCLK
GPIO26 (LRC)   →    LRC
3.3V           →    VIN
GND            →    GND
                 Speaker (+/-) on output terminals
```

**ESP32 → RC522 RFID Reader (SPI):**
```
ESP32-WROVER        RC522
GPIO21 (SDA)   →    SDA
GPIO23 (MOSI)  →    MOSI
GPIO19 (MISO)  →    MISO
GPIO18 (SCK)   →    SCK
3.3V           →    3.3V
GND            →    GND
GPIO22 (RST)   →    RST
```

**ESP32 → microSD Card Module (SPI):**
```
ESP32-WROVER        SD Module
GPIO15 (CS)    →    CS
GPIO13 (MOSI)  →    MOSI
GPIO16 (MISO)  →    MISO
GPIO14 (SCK)   →    SCK
5V or 3.3V     →    VCC
GND            →    GND
```

**ESP32 → Buttons (internal pull-up):**
```
ESP32-WROVER        Button
GPIO4          →    Next → GND
GPIO2          →    Previous → GND
GPIO0          →    Play/Pause → GND
```

**ESP32 → Rotary Encoder:**
```
ESP32-WROVER        KY-040
GPIO34 (CLK)   →    CLK
GPIO35 (DT)    →    DT
GPIO32 (SW)    →    SW (button)
3.3V           →    +
GND            →    GND
```

**ESP32 → NeoPixel Ring:**
```
ESP32-WROVER        NeoPixel Ring
GPIO12         →    DIN
5V or 3.3V     →    5V (works at 3.3V too)
GND            →    GND
```

---

## 💾 Software Setup

### No compiling needed (easiest):
1. Download **pre-built firmware** from: https://github.com/biologist79/ESPuino-Firmware
2. Flash to ESP32 using **ESP32 Flash Download Tool** (Windows) or esptool.py
3. Insert formatted microSD card
4. Power on — ESPuino creates a WiFi access point called "ESPuino"
5. Connect to it, captive portal appears
6. Enter home WiFi credentials
7. Web interface is now accessible at `http://espuino.local` (or the IP shown)

### Compiling yourself (for customisation):
1. Install **Visual Studio Code** + **PlatformIO** extension
2. Clone: `git clone https://github.com/biologist79/ESPuino.git`
3. Select environment (e.g. `custom` or your board)
4. Edit `src/settings-custom.h` with your GPIO assignments (use the wiring above)
5. Click "Upload and Monitor"

### First-time setup:
- Format microSD as FAT32
- Insert into ESPuino (mandatory — won't boot without it)
- ESPuino opens a WiFi AP named "ESPuino" on first boot
- Join it, enter your WiFi credentials via the captive portal
- Access web interface: browse to the IP shown in serial console or `http://espuino.local`
- Go to "Configuration" tab → set number of NeoPixel LEDs, volume limits, etc.
- Place an RFID tag on the reader, it appears in the web interface
- Browse to an audio folder on the SD card, assign it to the tag
- Done! Place the tag → audio plays

### Managing audio (three ways):
1. **Web interface upload** — upload MP3 files directly in the browser
2. **FTP** — enable via a button/modification card, connect with FileZilla (user: `esp32`, pass: `esp32`)
3. **Remove SD card** — classic way, plug into computer

---

## 🎮 Play Modes (assignable per NFC tag)

| Mode | What It Does |
|---|---|
| **Single track** | Plays one track once |
| **Single track (loop)** | Plays one track forever |
| **Audiobook** | Saves position when paused — perfect for stories! |
| **Audiobook (recursive)** | Same, including subdirectories |
| **Folder/playlist (sorted)** | Plays all tracks in order (album) |
| **Folder/playlist (random)** | Shuffle mode |
| **Webradio** | Plays an internet radio stream |
| **Random subdirectory** | Picks a random folder and plays |

### Modification Cards (special NFC tags that change behaviour):
- **Lock/unlock buttons** — for toddlers who smash buttons
- **Sleep timer** — 5/30/60/120 min or end of track/playlist
- **Dim NeoPixel** — night mode for bedtime stories
- **Loop track/playlist**
- **Toggle WiFi / Bluetooth**

---

## 🎨 Design Ideas for the Case

Since the rocket wasn't the direction — going with an **animal friend** face design instead. Perfect for a toddler.

### Animal Options:
1. **🦉 Owl** — Big round eyes = NeoPixel ring, wings = buttons, teal/brown colours
2. **🐻 Bear** — Chunky rounded body, ears as volume knobs, cream + brown
3. **🦊 Fox** — Pointy ears, orange + white colour scheme
4. **🐱 Cat** — Whiskers on the face, playful pastel colours
5. **🐶 Puppy** — Flappy ears as buttons, nose as speaker grille

### Key Design Requirements:
- Top surface must be **flat** with recess for NFC reader and magnets
- NFC coil ≈ 39mm diameter, magnet recess for 10x3mm magnets
- Speaker grille area (ventilation for sound) — could be the animal's mouth or belly
- Button holes for 30mm arcade buttons (hidden in sides or as paws)
- Access to USB-C (charging) — or a wireless charging pad built in
- NeoPixel ring visible (as collar, bow tie, or eyes)
- **Child-safe**: rounded corners, non-toxic PLA, no small parts

---

## 🗺️ Project Timeline

### Phase 1: Sourcing (Week 1)
- [ ] Order ESP32-WROVER (PSRAM! Don't forget)
- [ ] Order MAX98357A, RC522, SD card module
- [ ] Order arcade buttons, rotary encoder, NeoPixel ring
- [ ] Order NFC tags (NTAG213/215)
- [ ] Order PLA filament (2 colours)
- [ ] Download ESPuino firmware + tools

### Phase 2: Prototype (2-3 days)
- [ ] Breadboard all components
- [ ] Flash firmware, test audio playback
- [ ] Test NFC tag detection
- [ ] Test web interface (connect to WiFi)
- [ ] Test FTP upload
- [ ] Verify buttons + encoder work

### Phase 3: Design & Print (Week 2-3)
- [ ] Design rocket case in Fusion 360
- [ ] Test-fit print (all components)
- [ ] Iterate if needed
- [ ] Final print in chosen colours
- [ ] Post-process (sand)

### Phase 4: Assembly (1 day)
- [ ] Solder all connections on perfboard
- [ ] Mount components in case
- [ ] NeoPixel ring installation
- [ ] Magnets on top for Tonies
- [ ] Close case

### Phase 5: Content & Testing (1 day)
- [ ] Load SD card with songs + audiobooks
- [ ] Assign NFC tags via web interface
- [ ] Test all play modes
- [ ] Test WiFi upload from phone
- [ ] Kid-proof testing

### Phase 6: Gift Day! 🎁
- [ ] Print + assemble 2-3 custom Tonies (astronaut, planet, rocket)
- [ ] Write simple instructions for parents
- [ ] "WiFi audio player — open browser to add more stories!"

---

## 🛒 Shopping Links (UK Amazon)

| Part                               | Link                                                                   | Price   |
| ---------------------------------- | ---------------------------------------------------------------------- | ------- |
| ESP32-WROVER dev board (8MB PSRAM) | [Amazon UK](https://www.amazon.co.uk/dp/B087TF2L27)                    | ~£14    |
| MAX98357A I2S DAC                  | [Amazon UK](https://www.amazon.co.uk/s?k=max98357a)                    | ~£3-5   |
| RC522 RFID Kit (3-pack)            | [Amazon UK](https://www.amazon.co.uk/dp/B0821LQ96R)                    | ~£7     |
| microSD card module SPI            | [Amazon UK](https://www.amazon.co.uk/s?k=arduino+micro+sd+card+module) | ~£3-5   |
| NeoPixel Ring 12 LED               | [Amazon UK](https://www.amazon.co.uk/dp/B00KAE3R1U)                    | ~£7     |
| Arcade buttons 30mm × 3            | [Amazon UK](https://www.amazon.co.uk/s?k=30mm+arcade+buttons)          | ~£5-8   |
| Rotary Encoder KY-040              | [Amazon UK](https://www.amazon.co.uk/dp/B082MCZH8L)                    | ~£3     |
| Speaker 4-8 Ohm (Visaton FR 10)    | [Amazon UK](https://www.amazon.co.uk/s?k=Visaton+FR+10)                | ~£8-12  |
| NFC tags NTAG213 (pack)            | [Amazon UK](https://www.amazon.co.uk/s?k=ntag213)                      | ~£5-8   |
| Magnets 10×3mm neodymium           | [Amazon UK](https://www.amazon.co.uk/s?k=neodymium+magnets+10mm+x+3mm) | ~£3-5   |
| PLA Filament 1.75mm (2 spools)     | [Amazon UK](https://www.amazon.co.uk/s?k=PLA+filament+1.75mm)          | ~£12-15 |
| microSD Card 16GB Class 10         | [Amazon UK](https://www.amazon.co.uk/s?k=microsd+card+16gb+class+10)   | ~£5-8   |

💡 AliExpress is 30-50% cheaper vs Amazon but takes 2-4 weeks.

---

## ✨ Why ESP32 Wins for This Project

| Feature | Real Toniebox | Our ESPuino |
|---|---|---|
| Cost | £80+ | ~£40 |
| Custom audio | App only, limited | Drag & drop MP3 via WiFi |
| Languages | Only pre-recorded | Any language we record |
| Internet needed | Yes (cloud streaming) | No (SD card offline) |
| Parent control | App | Web interface + lock button |
| Screen time | None (good) | None (good) |
| Audiobook memory | Yes | Yes |
| Expandable | No | Add webradio, BT headphones |
| Open source | No | Fully open |

---

## 🔗 Key Resources

- **ESPuino GitHub:** https://github.com/biologist79/ESPuino
- **Pre-built Firmware:** https://github.com/biologist79/ESPuino-Firmware
- **ESPuino Forum:** https://forum.espuino.de (English section available)
- **ESPuino Web Interface Docs:** https://forum.espuino.de/t/dokumentation-webinterface/2807
- **Original Toniebox Teardown:** https://www.ifixit.com/Teardown/Toniebox+Teardown/106148
- **ESP32 Pinout Reference:** https://randomnerdtutorials.com/esp32-pinout-reference-gpios/
- **Biobox 3D Case:** https://forum.espuino.de/t/biobox-3d/3130
