#include "arduino_sd.h"

#include <SD.h>
#include <SPI.h>

namespace {
constexpr uint8_t kSdCsPin = 5;
constexpr uint8_t kSdSckPin = 18;
constexpr uint8_t kSdMisoPin = 19;
constexpr uint8_t kSdMosiPin = 23;
constexpr uint32_t kSpeeds[] = {4000000, 1000000, 400000};
constexpr const char* kMountPoint = "/sd";

bool spiStarted = false;

bool tryMount(bool formatIfEmpty) {
    if (!spiStarted) {
        SPI.begin(kSdSckPin, kSdMisoPin, kSdMosiPin, kSdCsPin);
        spiStarted = true;
    }

    for (size_t i = 0; i < sizeof(kSpeeds) / sizeof(kSpeeds[0]); ++i) {
        if (i > 0) {
            SD.end();
        }
        Serial.printf("SD: trying %u Hz%s...\n", static_cast<unsigned>(kSpeeds[i]),
                      formatIfEmpty ? " (format if needed)" : "");
        if (SD.begin(kSdCsPin, SPI, kSpeeds[i], kMountPoint, 5, formatIfEmpty)) {
            Serial.printf("SD: mounted at %u Hz\n", static_cast<unsigned>(kSpeeds[i]));
            return true;
        }
    }

    Serial.println("SD: mount failed at 4 MHz, 1 MHz and 400 kHz");
    return false;
}
}

bool mountSd() {
    return tryMount(false);
}

bool formatSd() {
    return tryMount(true);
}
