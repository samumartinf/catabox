#include "diagnostics.h"

#if defined(CATABOX_DIAG_SD)

#include <Arduino.h>
#include <SD.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "arduino_sd.h"

namespace {
bool mounted = false;
uint32_t lastTryMs = 0;
char line[128];
size_t lineLength = 0;
uint8_t buffer[512];

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

void printHelp() {
    Serial.println("Commands:");
    Serial.println("  l             list /stories");
    Serial.println("  f             format the card as FAT and create /stories");
    Serial.println("  u <name> <n>  upload n bytes as /stories/<name>");
    Serial.println("  ?             this help");
}

void printInfo() {
    Serial.printf("Card type: %s\n", cardTypeName(static_cast<uint8_t>(SD.cardType())));
    Serial.printf("Card size: %llu MB\n",
                  static_cast<unsigned long long>(SD.cardSize() / (1024ULL * 1024ULL)));
    Serial.printf("Free: %llu MB\n",
                  static_cast<unsigned long long>((SD.totalBytes() - SD.usedBytes()) /
                                                  (1024ULL * 1024ULL)));

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

void ensureStories() {
    if (!SD.exists("/stories")) {
        SD.mkdir("/stories");
    }
}

void uploadFile(const char* args) {
    char name[64] = {};
    unsigned long size = 0;
    if (std::sscanf(args, "%63s %lu", name, &size) != 2 || size == 0 || size > 64UL * 1024UL * 1024UL) {
        Serial.println("ERR usage: u <name> <bytes>");
        return;
    }
    if (std::strchr(name, '/') || std::strstr(name, "..")) {
        Serial.println("ERR bad name");
        return;
    }

    char path[96] = {};
    std::snprintf(path, sizeof(path), "/stories/%s", name);
    SD.remove(path);

    File file = SD.open(path, FILE_WRITE);
    if (!file) {
        Serial.println("ERR cannot create file");
        return;
    }

    Serial.println("READY");
    Serial.flush();
    Serial.setTimeout(30000);

    uint32_t remaining = static_cast<uint32_t>(size);
    uint32_t written = 0;
    uint32_t checksum = 0;
    bool ok = true;

    while (remaining > 0) {
        const size_t want = remaining < sizeof(buffer) ? remaining : sizeof(buffer);
        const size_t got = Serial.readBytes(buffer, want);
        if (got == 0 || file.write(buffer, got) != got) {
            ok = false;
            break;
        }
        for (size_t i = 0; i < got; ++i) {
            checksum += buffer[i];
        }
        written += got;
        remaining -= got;
    }

    file.close();
    if (!ok) {
        SD.remove(path);
        Serial.println("ERR upload failed");
        return;
    }

    Serial.printf("OK %u %08X\n", static_cast<unsigned>(written), static_cast<unsigned>(checksum));
    printInfo();
    Serial.println("LIST OK");
}

void doFormat() {
    Serial.println("FORMAT: formatting if needed...");
    mounted = formatSd();
    if (!mounted) {
        Serial.println("FORMAT FAILED");
        return;
    }
    ensureStories();
    printInfo();
    Serial.println("FORMAT OK");
}

void handleLine(char* text) {
    if (std::strcmp(text, "l") == 0) {
        if (!mounted) {
            Serial.println("ERR not mounted");
            return;
        }
        printInfo();
        Serial.println("LIST OK");
    } else if (std::strcmp(text, "f") == 0) {
        doFormat();
    } else if (std::strncmp(text, "u ", 2) == 0) {
        if (!mounted) {
            Serial.println("ERR not mounted");
            return;
        }
        uploadFile(text + 2);
    } else if (std::strcmp(text, "?") == 0) {
        printHelp();
    } else {
        Serial.println("ERR unknown command");
        printHelp();
    }
}

void pollSerial() {
    while (Serial.available() > 0) {
        const int c = Serial.read();
        if (c < 0) {
            break;
        }
        if (c == '\n' || c == '\r') {
            if (lineLength > 0) {
                line[lineLength] = '\0';
                handleLine(line);
                lineLength = 0;
            }
        } else if (lineLength < sizeof(line) - 1) {
            line[lineLength++] = static_cast<char>(c);
        }
    }
}
}

void diagSdBegin() {
    Serial.println();
    Serial.println("CataBox diagnostic: SD");
    mounted = mountSd();
    if (mounted) {
        ensureStories();
        printInfo();
    } else {
        Serial.println("Not mounted. Send 'f' to format the card as FAT.");
    }
    printHelp();
}

void diagSdLoop() {
    pollSerial();

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
        ensureStories();
        printInfo();
    }
}

#endif
