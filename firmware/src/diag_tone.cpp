#include "diagnostics.h"

#if defined(CATABOX_DIAG_TONE)

#include <Arduino.h>

namespace {
constexpr uint8_t kTonePin = 25;
constexpr uint8_t kChannel = 0;
bool on = false;
uint32_t lastToggleMs = 0;
}

void diagToneBegin() {
    Serial.println();
    Serial.println("CataBox diagnostic: tone");
    Serial.println("GPIO25 -> series resistor -> speaker -> GND");
    Serial.println("Use 100 ohm for a piezo, 220-330 ohm for a small dynamic speaker.");
    Serial.println("Never use 10 ohm directly from a GPIO.");

    ledcSetup(kChannel, 440, 8);
    ledcAttachPin(kTonePin, kChannel);
    ledcWrite(kChannel, 0);
    lastToggleMs = millis();
}

void diagToneLoop() {
    const uint32_t now = millis();
    const uint32_t interval = on ? 500 : 1500;
    if (now - lastToggleMs < interval) {
        return;
    }
    on = !on;
    ledcWrite(kChannel, on ? 128 : 0);
    lastToggleMs = now;
}

#endif
