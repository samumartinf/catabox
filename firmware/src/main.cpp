#include <Arduino.h>
#include <AudioLogger.h>

#include <cstring>

#include "app.h"
#include "arduino_bookmarks.h"
#include "arduino_controls.h"
#include "arduino_nfc.h"
#include "arduino_player.h"
#include "arduino_sd.h"
#include "config.h"
#include "diagnostics.h"

namespace {
constexpr uint8_t kLedPin = 2;
constexpr uint8_t kPausePin = 32;
constexpr uint8_t kRestartPin = 33;

ArduinoPlayer player;
ArduinoBookmarks bookmarks;
ArduinoNfc nfc;
Button pauseButton(kPausePin);
Button restartButton(kRestartPin);

bool sdReady = false;
uint32_t bootMs = 0;
uint32_t lastPollMs = 0;
uint32_t lastSdTryMs = 0;
uint32_t lastNoSdPollMs = 0;
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

void rememberPrintedUid(const char* uid) {
    std::strncpy(lastPrintedUid, uid, sizeof(lastPrintedUid) - 1);
    lastPrintedUid[sizeof(lastPrintedUid) - 1] = '\0';
}
}

void setup() {
    Serial.begin(115200);
    delay(100);
    Serial.println();
    Serial.println("CataBox v1");
    audioLogger = &Serial;

    pinMode(kLedPin, OUTPUT);

#if defined(CATABOX_DIAG_NFC)
    diagNfcBegin();
    return;
#elif defined(CATABOX_DIAG_I2C)
    diagI2cBegin();
    return;
#elif defined(CATABOX_DIAG_SD)
    diagSdBegin();
    return;
#elif defined(CATABOX_DIAG_AUDIO)
    diagAudioBegin();
    return;
#elif defined(CATABOX_DIAG_TONE)
    diagToneBegin();
    return;
#elif defined(CATABOX_DIAG_E2E)
    diagE2eBegin();
    return;
#endif

    sdReady = mountSd();

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

#if defined(CATABOX_DIAG_NFC)
    diagNfcLoop();
    delay(1);
    return;
#elif defined(CATABOX_DIAG_I2C)
    diagI2cLoop();
    delay(100);
    return;
#elif defined(CATABOX_DIAG_SD)
    diagSdLoop();
    delay(5);
    return;
#elif defined(CATABOX_DIAG_AUDIO)
    diagAudioLoop();
    delay(1);
    return;
#elif defined(CATABOX_DIAG_TONE)
    diagToneLoop();
    delay(1);
    return;
#elif defined(CATABOX_DIAG_E2E)
    diagE2eLoop();
    delay(1);
    return;
#endif

    if (!sdReady) {
        if (now - lastSdTryMs >= 5000) {
            lastSdTryMs = now;
            sdReady = mountSd();
        }

        // Keep the reader testable while the card is broken.
        if (now - lastNoSdPollMs >= 500) {
            lastNoSdPollMs = now;
            char uid[15];
            if (nfc.readUid(uid)) {
                if (std::strcmp(uid, lastPrintedUid) != 0) {
                    Serial.printf("UID (no SD): %s\n", uid);
                    rememberPrintedUid(uid);
                }
            } else {
                lastPrintedUid[0] = '\0';
            }
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
                rememberPrintedUid(uid);
            }
            app.onScan(uid, now);
        } else {
            lastPrintedUid[0] = '\0';
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
