#include "diagnostics.h"

#if defined(CATABOX_DIAG_E2E)

#include <Arduino.h>

#include <cstring>

#include "app.h"
#include "arduino_controls.h"
#include "arduino_nfc.h"

namespace {
constexpr uint8_t kTonePin = 25;
constexpr uint8_t kToneChannel = 0;
constexpr uint8_t kPausePin = 32;
constexpr uint8_t kRestartPin = 33;

// Plays a 440 Hz tone instead of an MP3, so the whole control path can be
// tested without the amplifier or SD card.
class TonePlayer : public catabox::IPlayer {
public:
    catabox::PlayResult play(const char* path, uint32_t startByte) override {
        (void)path;
        position_ = startByte;
        ledcWrite(kToneChannel, 128);
        playing_ = true;
        return catabox::PlayResult::Ok;
    }
    void stop() override {
        ledcWrite(kToneChannel, 0);
        playing_ = false;
    }
    bool service() override { return playing_; }
    uint32_t position() const override { return position_; }
    uint32_t size() const override { return 1000000; }
    bool atEnd() const override { return false; }
    uint32_t fileSize(const char*) override { return 1000000; }

private:
    bool playing_ = false;
    uint32_t position_ = 0;
};

class RamBookmarks : public catabox::IBookmarkStore {
public:
    uint32_t load(const char* uid) override {
        return (uid_[0] && std::strcmp(uid_, uid) == 0) ? position_ : 0;
    }
    void save(const char* uid, uint32_t position) override {
        std::strncpy(uid_, uid, sizeof(uid_) - 1);
        uid_[sizeof(uid_) - 1] = '\0';
        position_ = position;
    }
    void clear(const char* uid) override {
        if (uid_[0] && std::strcmp(uid_, uid) == 0) {
            uid_[0] = '\0';
            position_ = 0;
        }
    }

private:
    char uid_[15] = {};
    uint32_t position_ = 0;
};

ArduinoNfc nfc;
Button pauseButton(kPausePin);
Button restartButton(kRestartPin);
TonePlayer player;
RamBookmarks bookmarks;

void onError(const char* message) {
    Serial.printf("ERROR: %s\n", message);
}

catabox::App app(player, bookmarks, onError);
uint32_t lastPollMs = 0;
char lastUid[15] = {};
}

void diagE2eBegin() {
    Serial.println();
    Serial.println("CataBox diagnostic: end-to-end (tone)");
    Serial.println("Speaker on GPIO25 via 220-330 ohm to GND.");

    ledcSetup(kToneChannel, 440, 8);
    ledcAttachPin(kTonePin, kToneChannel);
    ledcWrite(kToneChannel, 0);

    if (!nfc.begin()) {
        Serial.println("PN532 init failed");
    }
    pauseButton.begin();
    restartButton.begin();
    Serial.println("Tap a tag: beep. Lift: silence. Replace: beep. Buttons: pause/restart.");
}

void diagE2eLoop() {
    const uint32_t now = millis();
    app.tick(now);

    if (now - lastPollMs >= 100) {
        lastPollMs = now;
        char uid[15];
        if (nfc.readUid(uid)) {
            if (std::strcmp(uid, lastUid) != 0) {
                Serial.printf("UID: %s\n", uid);
                std::strncpy(lastUid, uid, sizeof(lastUid) - 1);
                lastUid[sizeof(lastUid) - 1] = '\0';
            }
            app.onScan(uid, now);
        } else {
            lastUid[0] = '\0';
            app.onScan(nullptr, now);
        }
    }

    if (pauseButton.pressed(now)) {
        app.onPauseButton(now);
    }
    if (restartButton.pressed(now)) {
        app.onRestartButton(now);
    }
}

#endif
