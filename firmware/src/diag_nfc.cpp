#include "diagnostics.h"

#if defined(CATABOX_DIAG_NFC)

#include <Arduino.h>

#include <cstring>

#include "arduino_nfc.h"

namespace {
ArduinoNfc nfc;
uint32_t lastPollMs = 0;
char lastUid[15] = {};

void printUidOnce(const char* uid) {
    if (std::strcmp(uid, lastUid) == 0) {
        return;
    }
    Serial.printf("UID: %s\n", uid);
    std::strncpy(lastUid, uid, sizeof(lastUid) - 1);
    lastUid[sizeof(lastUid) - 1] = '\0';
}
}

void diagNfcBegin() {
    Serial.println();
    Serial.println("CataBox diagnostic: NFC");
    if (!nfc.begin()) {
        Serial.println("PN532 init failed");
        Serial.println("Check: DIP switches = I2C (SEL0 on, SEL1 off), SDA=21, SCL=22, VCC=3.3 V, GND");
        return;
    }
    Serial.printf("PN532 ready, firmware 0x%08X\n", static_cast<unsigned>(nfc.firmwareVersion()));
    Serial.println("Tap a tag...");
}

void diagNfcLoop() {
    const uint32_t now = millis();
    if (now - lastPollMs < 100) {
        return;
    }
    lastPollMs = now;

    char uid[15];
    if (nfc.readUid(uid)) {
        printUidOnce(uid);
    } else {
        lastUid[0] = '\0';
    }
}

#endif
