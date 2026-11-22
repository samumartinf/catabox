#include "diagnostics.h"

#if defined(CATABOX_DIAG_I2C)

#include <Arduino.h>
#include <Wire.h>

void diagI2cBegin() {
    Serial.println();
    Serial.println("CataBox diagnostic: I2C scan");
    Wire.begin(21, 22);

    int found = 0;
    for (uint8_t address = 1; address < 127; ++address) {
        Wire.beginTransmission(address);
        if (Wire.endTransmission() == 0) {
            Serial.printf("found 0x%02X\n", address);
            ++found;
        }
    }

    Serial.printf("scan done: %d device(s)\n", found);
    if (found == 0) {
        Serial.println("No I2C devices. Check VCC/GND, SDA=21, SCL=22, and DIP switches = I2C.");
    } else if (found != 1) {
        Serial.println("PN532 should be the only device at 0x24.");
    }
}

void diagI2cLoop() {}

#endif
