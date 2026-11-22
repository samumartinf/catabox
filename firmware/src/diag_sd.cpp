#include "diagnostics.h"

#if defined(CATABOX_DIAG_SD)

#include <Arduino.h>
#include <SD.h>

#include "arduino_sd.h"

namespace {
bool mounted = false;
uint32_t lastTryMs = 0;

const char* cardTypeName(uint8_t type) {
    switch (type) {
        case CARD_MMC:
            return "MMC";
        case CARD_SD:
            return "SDSC";
        case CARD_SDHC:
            return "SDHC";
        default:
            return "unknown";
    }
}

void printInfo() {
    Serial.printf("Card type: %s\n", cardTypeName(static_cast<uint8_t>(SD.cardType())));
    Serial.printf("Card size: %llu MB\n",
                  static_cast<unsigned long long>(SD.cardSize() / (1024ULL * 1024ULL)));

    File dir = SD.open("/stories");
    if (!dir) {
        Serial.println("/stories: not found");
        return;
    }
    if (!dir.isDirectory()) {
        Serial.println("/stories: not a directory");
        dir.close();
        return;
    }

    int entries = 0;
    for (File file = dir.openNextFile(); file; file = dir.openNextFile()) {
        Serial.printf("  %-32s %u bytes\n", file.name(), static_cast<unsigned>(file.size()));
        ++entries;
        file.close();
    }
    dir.close();
    Serial.printf("%d entries\n", entries);
}
}

void diagSdBegin() {
    Serial.println();
    Serial.println("CataBox diagnostic: SD");
    mounted = mountSd();
    if (mounted) {
        printInfo();
    }
}

void diagSdLoop() {
    if (mounted) {
        return;
    }
    const uint32_t now = millis();
    if (now - lastTryMs < 5000) {
        return;
    }
    lastTryMs = now;
    mounted = mountSd();
    if (mounted) {
        printInfo();
    }
}

#endif
