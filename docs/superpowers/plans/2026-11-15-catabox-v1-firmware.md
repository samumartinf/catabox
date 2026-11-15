# CataBox v1 Firmware Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build a minimal ESP32 firmware that plays a pre-recorded MP3 when a figurine's NFC tag is placed, pauses and remembers the spot when lifted, and has pause and restart buttons.

**Architecture:** A platform-independent core library holds the tag/presence state machine, bookmark rules, and UID→path helpers; it is host-tested with PlatformIO's native environment. Thin Arduino adapters wrap the PN532 reader, the SD/MP3 player, NVS bookmarks, and the two buttons. `main.cpp` polls hardware and feeds events into the core.

**Tech Stack:** ESP32 (Arduino framework), PlatformIO, ESP8266Audio (MP3 + I2S), Adafruit PN532 (I2C), Preferences (NVS), Unity host tests.

**Spec:** `docs/superpowers/specs/2026-11-15-catabox-v1-firmware-design.md`

## Global Constraints

- No WiFi, network, web UI, OTA, or Bluetooth code.
- One MP3 per figurine; path is `/stories/<UID>.mp3`, UID uppercase hex, no separators.
- Accept only 4-byte and 7-byte UIDs; log and ignore anything else.
- Tag absence debounce is 500 ms; poll the reader every 100 ms.
- Audio must pause within ~1 s of the figurine being lifted.
- Bookmark values are byte offsets; save every 10 s while playing, on pause, lift, and tag change; clear on natural end; keep on decode error.
- Resume only when the bookmark is at least 8 KB in and at least 8 KB before EOF.
- Restart and pause buttons only act while the loaded tag is present.
- Restart seeks to byte 0, clears the bookmark, clears user-pause, and plays.
- Fixed volume, `0.4` gain; no runtime volume control.
- Pins: I2S BCLK 27, LRC 26, DIN 25; I2C SDA 21, SCL 22; SD CS 5, SCK 18, MISO 19, MOSI 23; pause 32; restart 33; status LED 2.
- Core library (`lib/catabox_core`) must not include Arduino headers.
- Host tests run with `pio test -e native`; firmware builds with `pio run -e esp32dev`.

## Review Focus

- **Duplicate scans of the same tag** must not restart or re-open the story on every poll.
- **Short NFC dropouts (<500 ms)** must not pause audio or trigger a story swap.
- **A missing, empty, or unreadable MP3** must produce a handled error and leave the box alive, not a crash or reboot loop.
- **A bookmark pointing past EOF** (file replaced with a shorter one) must clamp to 0:00.
- **Two tags stacked or alternating** must not flap between stories while the current tag keeps being reported.

---

### Task 1: Project scaffold, UID helpers, bookmark math

**Files:**
- Create: `firmware/platformio.ini`
- Create: `firmware/lib/catabox_core/config.h`
- Create: `firmware/lib/catabox_core/uid.h`
- Create: `firmware/lib/catabox_core/uid.cpp`
- Create: `firmware/test/test_core/test_main.cpp`

**Interfaces:**
- Consumes: nothing.
- Produces:
  - `bool catabox::formatUid(const uint8_t* bytes, uint8_t length, char out[15])`
  - `void catabox::storyPath(const char* uid, char out[32])`
  - `uint32_t catabox::resumePosition(uint32_t bookmark, uint32_t fileSize)`

- [ ] **Step 1: Write the failing tests**

In `firmware/test/test_core/test_main.cpp`:

```cpp
#include <unity.h>
#include "uid.h"

void test_format_7_byte_uid() {
    const uint8_t uid[] = {0x04, 0xA3, 0xB2, 0xC1, 0xD4, 0xE5, 0x80};
    char out[15];
    TEST_ASSERT_TRUE(catabox::formatUid(uid, 7, out));
    TEST_ASSERT_EQUAL_STRING("04A3B2C1D4E580", out);
}

void test_format_4_byte_uid() {
    const uint8_t uid[] = {0xDE, 0xAD, 0xBE, 0xEF};
    char out[15];
    TEST_ASSERT_TRUE(catabox::formatUid(uid, 4, out));
    TEST_ASSERT_EQUAL_STRING("DEADBEEF", out);
}

void test_reject_other_uid_lengths() {
    const uint8_t uid[10] = {0};
    char out[15];
    TEST_ASSERT_FALSE(catabox::formatUid(uid, 0, out));
    TEST_ASSERT_FALSE(catabox::formatUid(uid, 10, out));
}

void test_story_path() {
    char out[32];
    catabox::storyPath("04A3B2C1D4E580", out);
    TEST_ASSERT_EQUAL_STRING("/stories/04A3B2C1D4E580.mp3", out);
}

void test_resume_position_rules() {
    TEST_ASSERT_EQUAL_UINT32(0, catabox::resumePosition(0, 100000));
    TEST_ASSERT_EQUAL_UINT32(0, catabox::resumePosition(5000, 100000));
    TEST_ASSERT_EQUAL_UINT32(50000, catabox::resumePosition(50000, 100000));
    TEST_ASSERT_EQUAL_UINT32(0, catabox::resumePosition(95000, 100000));
    TEST_ASSERT_EQUAL_UINT32(0, catabox::resumePosition(50000, 0));
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_format_7_byte_uid);
    RUN_TEST(test_format_4_byte_uid);
    RUN_TEST(test_reject_other_uid_lengths);
    RUN_TEST(test_story_path);
    RUN_TEST(test_resume_position_rules);
    return UNITY_END();
}
```

- [ ] **Step 2: Run the tests and confirm they fail**

Run: `cd firmware && ~/.platformio/penv/bin/pio test -e native`
Expected: compile fails because `uid.h` does not exist.

- [ ] **Step 3: Create `firmware/platformio.ini`**

```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200
lib_deps =
    earlephilhower/ESP8266Audio
    adafruit/Adafruit PN532

[env:native]
platform = native
test_framework = unity
build_flags = -std=gnu++17
lib_compat_mode = off
```

- [ ] **Step 4: Create `config.h` and `uid.h`**

`config.h` pins the numbers from the spec:

```cpp
#pragma once
#include <cstdint>

namespace catabox {
constexpr uint32_t kAbsenceMs = 500;
constexpr uint32_t kPollMs = 100;
constexpr uint32_t kSaveIntervalMs = 10000;
constexpr uint32_t kSaveMinBytes = 32768;
constexpr uint32_t kResumeMinBytes = 8192;
constexpr uint32_t kResumeEndMargin = 8192;
constexpr uint32_t kBootDelayMs = 1000;
constexpr uint32_t kButtonDebounceMs = 40;
constexpr float kVolume = 0.4f;
constexpr const char* kStoriesDir = "/stories/";
}
```

`uid.h`:

```cpp
#pragma once
#include <cstdint>

namespace catabox {
bool formatUid(const uint8_t* bytes, uint8_t length, char out[15]);
void storyPath(const char* uid, char out[32]);
uint32_t resumePosition(uint32_t bookmark, uint32_t fileSize);
}
```

- [ ] **Step 5: Implement `uid.cpp`**

Format each byte as two uppercase hex digits; return false unless length is 4 or 7. `storyPath` writes `"/stories/" + uid + ".mp3"`. `resumePosition` returns 0 when `fileSize == 0`, when `bookmark < kResumeMinBytes`, or when `bookmark + kResumeEndMargin >= fileSize`; otherwise returns `bookmark`.

- [ ] **Step 6: Run tests and confirm they pass**

Run: `cd firmware && ~/.platformio/penv/bin/pio test -e native`
Expected: 5 tests pass.

- [ ] **Step 7: Commit**

```bash
git add firmware/platformio.ini firmware/lib/catabox_core/config.h firmware/lib/catabox_core/uid.h firmware/lib/catabox_core/uid.cpp firmware/test/test_core/test_main.cpp
git commit -m "feat(firmware): scaffold PlatformIO and UID/bookmark helpers"
```

---

### Task 2: Core state machine — playback, lift, resume, pause

**Files:**
- Create: `firmware/lib/catabox_core/player.h`
- Create: `firmware/lib/catabox_core/bookmarks.h`
- Create: `firmware/lib/catabox_core/app.h`
- Create: `firmware/lib/catabox_core/app.cpp`
- Modify: `firmware/test/test_core/test_main.cpp`

**Interfaces:**
- Consumes: `formatUid`, `storyPath`, `resumePosition`, constants from `config.h`.
- Produces:
  - `enum class catabox::PlayResult { Ok, FileMissing, Error };`
  - `class catabox::IPlayer` with `play(const char*, uint32_t)`, `stop()`, `service()`, `position()`, `size()`, `atEnd()`, `fileSize(const char*)`
  - `class catabox::IBookmarkStore` with `load(uid)`, `save(uid, uint32_t)`, `clear(uid)`
  - `class catabox::App` with `onScan(const char* uidOrNull, uint32_t nowMs)`, `onPauseButton(uint32_t)`, `onRestartButton(uint32_t)`, `tick(uint32_t)`, `state()`, `tagPresent()`, `userPaused()`, `loadedUid()`, `lastError()`

- [ ] **Step 1: Write the failing tests**

Add fakes and tests to `test_main.cpp`:

```cpp
#include <cstring>
#include <map>
#include <string>
#include "app.h"

class FakePlayer : public catabox::IPlayer {
public:
    std::string lastPath;
    uint32_t lastStart = 0;
    int playCalls = 0;
    int stopCalls = 0;
    uint32_t pos = 0;
    uint32_t sz = 100000;
    bool serviceResult = true;
    bool endFlag = false;
    catabox::PlayResult nextResult = catabox::PlayResult::Ok;

    catabox::PlayResult play(const char* path, uint32_t start) override {
        lastPath = path; lastStart = start; playCalls++;
        pos = start; endFlag = false; return nextResult;
    }
    void stop() override { stopCalls++; }
    bool service() override { return serviceResult; }
    uint32_t position() const override { return pos; }
    uint32_t size() const override { return sz; }
    bool atEnd() const override { return endFlag; }
    uint32_t fileSize(const char*) override { return sz; }
};

class FakeBookmarks : public catabox::IBookmarkStore {
public:
    std::map<std::string, uint32_t> values;
    uint32_t load(const char* uid) override {
        auto it = values.find(uid); return it == values.end() ? 0 : it->second;
    }
    void save(const char* uid, uint32_t pos) override { values[uid] = pos; }
    void clear(const char* uid) override { values.erase(uid); }
};

static const char* kUid = "04A3B2C1D4E580";

void test_tag_plays_story_from_start() {
    FakePlayer player; FakeBookmarks marks; catabox::App app(player, marks);
    app.onScan(kUid, 0);
    TEST_ASSERT_EQUAL_STRING("/stories/04A3B2C1D4E580.mp3", player.lastPath.c_str());
    TEST_ASSERT_EQUAL_UINT32(0, player.lastStart);
    TEST_ASSERT_TRUE(app.state() == catabox::AppState::Playing);
    TEST_ASSERT_TRUE(app.tagPresent());
}

void test_repeated_scans_do_not_restart() {
    FakePlayer player; FakeBookmarks marks; catabox::App app(player, marks);
    app.onScan(kUid, 0);
    app.onScan(kUid, 100);
    app.onScan(kUid, 200);
    TEST_ASSERT_EQUAL_INT(1, player.playCalls);
}

void test_short_dropout_is_ignored() {
    FakePlayer player; FakeBookmarks marks; catabox::App app(player, marks);
    app.onScan(kUid, 0);
    app.onScan(nullptr, 100);
    app.onScan(kUid, 200);
    TEST_ASSERT_EQUAL_INT(1, player.playCalls);
    TEST_ASSERT_EQUAL_INT(0, player.stopCalls);
    TEST_ASSERT_TRUE(app.state() == catabox::AppState::Playing);
}

void test_lift_pauses_and_saves() {
    FakePlayer player; FakeBookmarks marks; catabox::App app(player, marks);
    app.onScan(kUid, 0);
    player.pos = 40000;
    app.onScan(nullptr, 600);
    TEST_ASSERT_EQUAL_INT(1, player.stopCalls);
    TEST_ASSERT_EQUAL_UINT32(40000, marks.values[kUid]);
    TEST_ASSERT_FALSE(app.tagPresent());
    TEST_ASSERT_TRUE(app.state() == catabox::AppState::Paused);
}

void test_return_resumes_at_saved_position() {
    FakePlayer player; FakeBookmarks marks; catabox::App app(player, marks);
    app.onScan(kUid, 0);
    player.pos = 40000;
    app.onScan(nullptr, 600);
    app.onScan(kUid, 700);
    TEST_ASSERT_EQUAL_UINT32(40000, player.lastStart);
    TEST_ASSERT_TRUE(app.state() == catabox::AppState::Playing);
}

void test_user_pause_survives_lift_and_return() {
    FakePlayer player; FakeBookmarks marks; catabox::App app(player, marks);
    app.onScan(kUid, 0);
    player.pos = 30000;
    app.onPauseButton(100);
    TEST_ASSERT_TRUE(app.userPaused());
    app.onScan(nullptr, 700);
    app.onScan(kUid, 800);
    TEST_ASSERT_EQUAL_INT(1, player.playCalls);
    TEST_ASSERT_TRUE(app.userPaused());
    app.onPauseButton(900);
    TEST_ASSERT_EQUAL_UINT32(30000, player.lastStart);
    TEST_ASSERT_TRUE(app.state() == catabox::AppState::Playing);
}
```

Add the new `RUN_TEST(...)` lines to `main`.

- [ ] **Step 2: Run tests and confirm they fail**

Run: `cd firmware && ~/.platformio/penv/bin/pio test -e native`
Expected: compile fails because `app.h` and the core classes do not exist.

- [ ] **Step 3: Define the interfaces**

`player.h` and `bookmarks.h` contain only the abstract interfaces above plus `#include <cstdint>`.

`app.h`:

```cpp
#pragma once
#include <cstdint>
#include "player.h"
#include "bookmarks.h"

namespace catabox {
enum class AppState { Idle, Playing, Paused };
using ErrorCallback = void (*)(const char* message);

class App {
public:
    App(IPlayer& player, IBookmarkStore& bookmarks, ErrorCallback onError = nullptr);
    void onScan(const char* uidOrNull, uint32_t nowMs);
    void onPauseButton(uint32_t nowMs);
    void onRestartButton(uint32_t nowMs);
    void tick(uint32_t nowMs);
    AppState state() const;
    bool tagPresent() const;
    bool userPaused() const;
    const char* loadedUid() const;
    const char* lastError() const;
};
}
```

- [ ] **Step 4: Implement the state machine**

Use `char selected_[15]`, `char loaded_[15]`, `char finished_[15]`, `char failed_[15]`, `char path_[32]`, `uint32_t lastSeenMs_`, `lastSavedMs_`, `lastSavedPos_`, `pausedPos_`, `bool tagPresent_`, `bool userPaused_`, `bool playing_`.

Rules to implement now (latches come in Task 3, but leave the fields in place):

- `onScan(uid, now)`: empty uid → if selected and `now - lastSeenMs_ >= kAbsenceMs`, clear selected; if tag was present, mark absent and pause/save. Same uid as selected → refresh `lastSeenMs_`; if it had been marked absent, start the story again. Different uid → ignore while `now - lastSeenMs_ < kAbsenceMs`; otherwise treat the old tag as absent, then select the new one.
- Selecting a tag calls `startStory(uid)`. If the UID differs from `loaded_`, stop playback and save the old bookmark first; if the UID is the same, do **not** reset `userPaused_`. If `userPaused_` is true for the same UID, do nothing. Otherwise compute the path, call `fileSize`, apply `resumePosition`, set `pausedPos_ = start`, and call `player.play(path, start)`. `PlayResult::FileMissing`/`Error` → set `lastError_`, call the error callback, stay not-playing.
- Lifting: `pausedPos_ = player.position()`, `player.stop()`, `playing_ = false`, save the bookmark.
- `onPauseButton`: only when `tagPresent_` and a story is loaded and not latched. Toggle `userPaused_`; pausing stops and saves, unpausing calls `player.play(path_, pausedPos_)`.
- `tick`: if playing, call `player.service()`. If it returns false, stop, set `pausedPos_` to the reported position, and leave the state Idle (Task 3 adds finish/error handling). Every `kSaveIntervalMs`, if the position advanced by `kSaveMinBytes`, save it.
- `state()` returns Playing while `playing_`; Paused when `loaded_` is set, not playing, and not latched; otherwise Idle.

- [ ] **Step 5: Run tests and confirm they pass**

Run: `cd firmware && ~/.platformio/penv/bin/pio test -e native`
Expected: 11 tests pass.

- [ ] **Step 6: Commit**

```bash
git add firmware/lib/catabox_core firmware/test/test_core/test_main.cpp
git commit -m "feat(firmware): core playback state machine with host tests"
```

---

### Task 3: Core edge cases — tag swap, restart, finish/error latches

**Files:**
- Modify: `firmware/lib/catabox_core/app.cpp`
- Modify: `firmware/test/test_core/test_main.cpp`

**Interfaces:**
- Consumes: everything from Task 2.
- Produces: same public `App` API; added behaviour only.

- [ ] **Step 1: Write the failing tests**

Add:

```cpp
void test_different_tag_while_current_still_seen_is_ignored() {
    FakePlayer player; FakeBookmarks marks; catabox::App app(player, marks);
    app.onScan(kUid, 0);
    app.onScan("DEADBEEF", 100);   // current tag seen 100 ms ago
    TEST_ASSERT_EQUAL_INT(1, player.playCalls);
    TEST_ASSERT_EQUAL_STRING(kUid, app.loadedUid());
}

void test_different_tag_after_absence_swaps_story() {
    FakePlayer player; FakeBookmarks marks; catabox::App app(player, marks);
    app.onScan(kUid, 0);
    player.pos = 25000;
    app.onScan("DEADBEEF", 600);
    TEST_ASSERT_EQUAL_UINT32(25000, marks.values[kUid]);
    TEST_ASSERT_EQUAL_STRING("/stories/DEADBEEF.mp3", player.lastPath.c_str());
    TEST_ASSERT_EQUAL_UINT32(0, player.lastStart);
}

void test_restart_needs_tag_present() {
    FakePlayer player; FakeBookmarks marks; catabox::App app(player, marks);
    app.onScan(kUid, 0);
    app.onScan(nullptr, 600);
    int calls = player.playCalls;
    app.onRestartButton(700);
    TEST_ASSERT_EQUAL_INT(calls, player.playCalls);
}

void test_restart_rewinds_and_clears_bookmark() {
    FakePlayer player; FakeBookmarks marks; catabox::App app(player, marks);
    app.onScan(kUid, 0);
    marks.values[kUid] = 50000;
    app.onRestartButton(100);
    TEST_ASSERT_EQUAL_UINT32(0, player.lastStart);
    TEST_ASSERT_EQUAL_UINT32(0, static_cast<uint32_t>(marks.values.count(kUid)));
    TEST_ASSERT_TRUE(app.state() == catabox::AppState::Playing);
}

void test_finish_clears_bookmark_and_does_not_replay() {
    FakePlayer player; FakeBookmarks marks; catabox::App app(player, marks);
    app.onScan(kUid, 0);
    marks.values[kUid] = 60000;
    player.pos = 100000; player.endFlag = true; player.serviceResult = false;
    app.tick(100);
    TEST_ASSERT_EQUAL_UINT32(0, static_cast<uint32_t>(marks.values.count(kUid)));
    int calls = player.playCalls;
    app.onScan(kUid, 200);
    app.tick(300);
    app.onScan(kUid, 400);
    TEST_ASSERT_EQUAL_INT(calls, player.playCalls);
}

void test_finished_tag_replays_after_removal() {
    FakePlayer player; FakeBookmarks marks; catabox::App app(player, marks);
    app.onScan(kUid, 0);
    player.pos = 100000; player.endFlag = true; player.serviceResult = false;
    app.tick(100);
    app.onScan(nullptr, 700);
    app.onScan(kUid, 800);
    TEST_ASSERT_EQUAL_UINT32(0, player.lastStart);
    TEST_ASSERT_TRUE(app.state() == catabox::AppState::Playing);
}

void test_decode_error_keeps_bookmark_and_latches() {
    FakePlayer player; FakeBookmarks marks; catabox::App app(player, marks);
    app.onScan(kUid, 0);
    player.pos = 30000; player.endFlag = false; player.serviceResult = false;
    app.tick(100);
    TEST_ASSERT_EQUAL_UINT32(30000, marks.values[kUid]);
    int calls = player.playCalls;
    app.onScan(kUid, 200);
    TEST_ASSERT_EQUAL_INT(calls, player.playCalls);
}

void test_missing_file_is_handled_and_retried_after_removal() {
    FakePlayer player; FakeBookmarks marks; catabox::App app(player, marks);
    player.nextResult = catabox::PlayResult::FileMissing;
    app.onScan(kUid, 0);
    TEST_ASSERT_TRUE(app.state() == catabox::AppState::Idle);
    TEST_ASSERT_TRUE(app.lastError()[0] != '\0');
    int calls = player.playCalls;
    app.onScan(kUid, 100);
    TEST_ASSERT_EQUAL_INT(calls, player.playCalls);
    app.onScan(nullptr, 700);
    player.nextResult = catabox::PlayResult::Ok;
    app.onScan(kUid, 800);
    TEST_ASSERT_EQUAL_INT(calls + 1, player.playCalls);
}

void test_audio_open_error_is_handled_and_latched() {
    FakePlayer player; FakeBookmarks marks; catabox::App app(player, marks);
    player.nextResult = catabox::PlayResult::Error;
    app.onScan(kUid, 0);
    TEST_ASSERT_TRUE(app.state() == catabox::AppState::Idle);
    TEST_ASSERT_TRUE(app.lastError()[0] != '\0');
    int calls = player.playCalls;
    app.onScan(kUid, 100);
    TEST_ASSERT_EQUAL_INT(calls, player.playCalls);
    app.onScan(nullptr, 700);
    player.nextResult = catabox::PlayResult::Ok;
    app.onScan(kUid, 800);
    TEST_ASSERT_EQUAL_INT(calls + 1, player.playCalls);
}

void test_button_without_tag_does_nothing() {
    FakePlayer player; FakeBookmarks marks; catabox::App app(player, marks);
    app.onPauseButton(0);
    app.onRestartButton(0);
    TEST_ASSERT_EQUAL_INT(0, player.playCalls);
}

void test_short_tag_bounce_does_not_swap() {
    FakePlayer player; FakeBookmarks marks; catabox::App app(player, marks);
    app.onScan(kUid, 0);
    app.onScan("DEADBEEF", 300);   // only 300 ms since current tag was seen
    app.onScan(kUid, 400);
    TEST_ASSERT_EQUAL_STRING(kUid, app.loadedUid());
    TEST_ASSERT_EQUAL_INT(1, player.playCalls);
}
```

Add the new `RUN_TEST(...)` lines to `main`.

- [ ] **Step 2: Run tests and confirm they fail**

Run: `cd firmware && ~/.platformio/penv/bin/pio test -e native`
Expected: failures for the new cases (swap, restart, latches).

- [ ] **Step 3: Implement the edge cases**

- **Different tag:** only swap when `now - lastSeenMs_ >= kAbsenceMs`; save the old bookmark first.
- **Restart:** guard on `tagPresent_`, a loaded story, and no latch; clear bookmark, set `userPaused_ = false`, `player.play(path_, 0)`.
- **Finish:** when `service()` is false and `player.atEnd()`, stop, clear the bookmark, copy the UID into `finished_`.
- **Error:** when `service()` is false and not `atEnd()`, stop, save the current position, copy the UID into `failed_`.
- **Latches:** while `selected_` equals `finished_` or `failed_`, do not start. When a tag becomes absent, clear both latch strings (they only ever hold the loaded UID).
- **Missing file:** `play` returning `FileMissing` sets `failed_`, reports, and keeps the box idle.
- **Swaps:** starting a new story stops the current player if it is playing and saves the old bookmark.

- [ ] **Step 4: Run tests and confirm they pass**

Run: `cd firmware && ~/.platformio/penv/bin/pio test -e native`
Expected: all 22 tests pass.

- [ ] **Step 5: Commit**

```bash
git add firmware/lib/catabox_core/app.cpp firmware/test/test_core/test_main.cpp
git commit -m "feat(firmware): tag swap, restart, finish and error handling"
```

---

### Task 4: Arduino adapters and firmware entry point

**Files:**
- Create: `firmware/src/arduino_player.h`
- Create: `firmware/src/arduino_player.cpp`
- Create: `firmware/src/arduino_bookmarks.h`
- Create: `firmware/src/arduino_bookmarks.cpp`
- Create: `firmware/src/arduino_nfc.h`
- Create: `firmware/src/arduino_nfc.cpp`
- Create: `firmware/src/arduino_controls.h`
- Create: `firmware/src/arduino_controls.cpp`
- Create: `firmware/src/main.cpp`

**Interfaces:**
- Consumes: `catabox::App`, `IPlayer`, `IBookmarkStore`, `config.h`, `uid.h`.
- Produces: a flashable firmware image; no host-testable API.

- [ ] **Step 1: Implement `ArduinoPlayer`**

- Constructor allocates `AudioOutputI2S`, calls `SetPinout(27, 26, 25)` and `SetGain(catabox::kVolume)`.
- `play(path, start)`: stop any current playback, reset `atEnd_`, create `AudioFileSourceSD`; if `!isOpen()` return `FileMissing`; seek to `start` when non-zero; create `AudioGeneratorMP3`; `begin(source, output)`; on failure stop and return `Error`.
- `service()`: return false when no generator is running; otherwise call `gen->loop()`. When it returns false, cache `atEnd_ = position() + 4096 >= size()`.
- `position()`/`size()` delegate to the source (0 when absent). `fileSize(path)` opens the file with `SD.open`, reads `size()`, closes, returns it.
- `stop()` stops and deletes the generator, closes and deletes the source, but leaves `atEnd_` intact.

- [ ] **Step 2: Implement `ArduinoBookmarks`**

Use `Preferences` with namespace `"catabox"`; open read-only for `load`, read-write for `save`/`clear`; keys are the UID strings. `putUInt`/`getUInt`/`remove`.

- [ ] **Step 3: Implement `ArduinoNfc`**

`Wire.begin(21, 22)`; `Adafruit_PN532 nfc_{255, 255}` (255 maps to “unused” IRQ/reset); `begin()` calls `nfc_.begin()` then `SAMConfig()`. `readUid(char out[15])` calls `readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &len, 50)` and `catabox::formatUid`; returns false for no tag or an unsupported UID length.

- [ ] **Step 4: Implement `Button`**

Active-low with `INPUT_PULLUP`. `pressed(now)` returns true on the debounced release edge (`kButtonDebounceMs`), once per press.

- [ ] **Step 5: Implement `main.cpp`**

- `setup`: Serial 115200; LED pin output; `Wire.begin`; `SPI.begin(18, 19, 23, 5)`; mount SD with `SD.begin(5, SPI)`; init NFC and buttons; record `bootMs`.
- `loop`: while SD is not mounted, retry every 5 s and blink slowly; once mounted and `millis() - bootMs >= kBootDelayMs`, poll NFC every `kPollMs`, feed `onScan(uid or nullptr)`, feed button events, and call `app.tick(millis())` every iteration. Print UID on first read and every error through the app callback.
- Non-blocking blink counter for errors (3 short flashes); no `delay()` over 10 ms.

- [ ] **Step 6: Build the firmware**

Run: `cd firmware && ~/.platformio/penv/bin/pio run -e esp32dev`
Expected: SUCCESS, firmware linked for `esp32dev`.

- [ ] **Step 7: Commit**

```bash
git add firmware/src
git commit -m "feat(firmware): ESP32 adapters and entry point"
```

---

### Task 5: Wiring diagram, firmware README, project docs

**Files:**
- Create: `docs/wiring/catabox-wiring.svg`
- Create: `firmware/README.md`
- Modify: `README.md`
- Modify: `docs/SPEC.md`
- Modify: `docs/ROADMAP.md`
- Modify: `docs/DECISIONS.md`

**Interfaces:**
- Consumes: the pin map from the spec.
- Produces: build/flash/content instructions and a visual wiring reference.

- [ ] **Step 1: Draw the wiring diagram**

SVG with labelled blocks for ESP32 devkit, PN532, MAX98357A, SD module, two buttons, speaker, and 5 V supply. Colour-code: red 5 V, orange 3.3 V, black GND, green I2S, blue I2C, purple SPI, grey buttons. Label every pin with its GPIO number. Include a note: PN532 DIP switches to I2C, MAX98357A VIN to 5 V, common ground, 470 µF across amp VIN/GND.

- [ ] **Step 2: Write `firmware/README.md`**

Cover: install PlatformIO, `pio run -e esp32dev -t upload`, `pio test -e native`, SD card layout (`/stories/<UID>.mp3`), how to read a UID from the serial monitor, how to rename/copy a story, controls, the stacked-tag limitation, and that volume is set in `config.h`.

- [ ] **Step 3: Update project docs**

- `README.md`: status → Phase 1 firmware written; point at the firmware README and wiring SVG.
- `docs/SPEC.md`: v0.3, custom minimal firmware, ESPuino moved to “rejected for v1 (ADR-007)”.
- `docs/DECISIONS.md`: append ADR-007 (custom firmware, with the reasons and consequences), and mark ADR-005 superseded.
- `docs/ROADMAP.md`: tick the firmware work; leave hardware bring-up items open.

- [ ] **Step 4: Check the tree builds and tests pass**

Run: `cd firmware && ~/.platformio/penv/bin/pio test -e native && ~/.platformio/penv/bin/pio run -e esp32dev`
Expected: tests pass, firmware builds.

- [ ] **Step 5: Commit and push**

```bash
git add docs firmware README.md
git commit -m "docs: wiring diagram, firmware README, v0.3 project docs"
git push origin main
```
