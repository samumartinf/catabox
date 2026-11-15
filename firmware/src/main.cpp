#include <Arduino.h>
#include <AudioLogger.h>
#include <SD.h>
#include <SPI.h>
#include <Wire.h>

#include <cstring>

#include "app.h"
#include "arduino_bookmarks.h"
#include "arduino_controls.h"
#include "arduino_nfc.h"
#include "arduino_player.h"
#include "config.h"

namespace {
constexpr uint8_t kLedPin = 2;
constexpr uint8_t kPausePin = 32;
constexpr uint8_t kRestartPin = 33;
constexpr uint8_t kSdCsPin = 5;
constexpr uint8_t kSdSckPin = 18;
constexpr uint8_t kSdMisoPin = 19;
constexpr uint8_t kSdMosiPin = 23;

ArduinoPlayer player;
ArduinoBookmarks bookmarks;
ArduinoNfc nfc;
Button pauseButton(kPausePin);
Button restartButton(kRestartPin);

bool sdReady = false;
uint32_t bootMs = 0;
uint32_t lastPollMs = 0;
uint32_t lastSdTryMs = 0;
uint32_t blinkUntilMs = 0;
char lastPrintedUid[15] = {};

void onAppError(const char* message) {
    Serial.printf("ERROR: %s\n", message);
    blinkUntilMs = millis() + 600;
}

catabox::App app(player, bookmarks, onAppError);

void updateLed(uint32_t now) {
    bool on = false;
    if (!sdReady) {
        on = (now / 500) % 2 == 0;
    } else if (now < blinkUntilMs) {
        on = (now / 100) % 2 == 0;
    }
    digitalWrite(kLedPin, on ? HIGH : LOW);
}
}

void setup() {
    Serial.begin(115200);
    delay(100);
    Serial.println();
    Serial.println("CataBox v1");
    audioLogger = &Serial;

    pinMode(kLedPin, OUTPUT);
    SPI.begin(kSdSckPin, kSdMisoPin, kSdMosiPin, kSdCsPin);

    sdReady = SD.begin(kSdCsPin, SPI, 4000000);
    Serial.println(sdReady ? "SD mounted" : "SD mount failed");

    if (!nfc.begin()) {
        Serial.println("PN532 init failed");
    }

    pauseButton.begin();
    restartButton.begin();
    bootMs = millis();
    lastSdTryMs = bootMs;
}

void loop() {
    const uint32_t now = millis();

    if (!sdReady) {
        if (now - lastSdTryMs >= 5000) {
            lastSdTryMs = now;
            sdReady = SD.begin(kSdCsPin, SPI, 4000000);
            Serial.println(sdReady ? "SD mounted" : "SD mount failed");
        }
        updateLed(now);
        delay(5);
        return;
    }

    if (now - bootMs < catabox::kBootDelayMs) {
        updateLed(now);
        delay(5);
        return;
    }

    app.tick(now);

    if (now - lastPollMs >= catabox::kPollMs) {
        lastPollMs = now;

        char uid[15];
        if (nfc.readUid(uid)) {
            if (std::strcmp(uid, lastPrintedUid) != 0) {
                Serial.printf("UID: %s\n", uid);
                std::strncpy(lastPrintedUid, uid, sizeof(lastPrintedUid) - 1);
                lastPrintedUid[sizeof(lastPrintedUid) - 1] = '\0';
            }
            app.onScan(uid, now);
        } else {
            app.onScan(nullptr, now);
        }
    }

    if (pauseButton.pressed(now)) {
        app.onPauseButton(now);
    }
    if (restartButton.pressed(now)) {
        app.onRestartButton(now);
    }

    updateLed(now);
    delay(1);
}
