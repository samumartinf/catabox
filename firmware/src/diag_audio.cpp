#include "diagnostics.h"

#if defined(CATABOX_DIAG_AUDIO)

#include <Arduino.h>
#include <SD.h>

#include <cstdio>
#include <cstring>

#include "arduino_player.h"
#include "arduino_sd.h"
#include "media.h"

namespace {
ArduinoPlayer player;
bool mounted = false;
bool playing = false;
uint32_t lastTryMs = 0;
uint32_t lastPrintMs = 0;
char path[64] = {};

bool pickFile() {
    const char* preferred = "/stories/diag.mp3";
    File preferredFile = SD.open(preferred);
    if (preferredFile) {
        preferredFile.close();
        std::strncpy(path, preferred, sizeof(path) - 1);
        path[sizeof(path) - 1] = '\0';
        return true;
    }

    File dir = SD.open("/stories");
    if (!dir || !dir.isDirectory()) {
        if (dir) {
            dir.close();
        }
        return false;
    }

    for (File file = dir.openNextFile(); file; file = dir.openNextFile()) {
        if (!file.isDirectory() && catabox::hasMp3Extension(file.name())) {
            const char* name = file.name();
            if (name[0] == '/') {
                std::snprintf(path, sizeof(path), "%s", name);
            } else {
                std::snprintf(path, sizeof(path), "/stories/%s", name);
            }
            file.close();
            dir.close();
            return true;
        }
        file.close();
    }
    dir.close();
    return false;
}

void startPlayback() {
    if (!pickFile()) {
        Serial.println("No MP3 found. Put one at /stories/diag.mp3 or any .mp3 in /stories.");
        return;
    }

    Serial.printf("Playing %s\n", path);
    const catabox::PlayResult result = player.play(path, 0);
    if (result != catabox::PlayResult::Ok) {
        Serial.println(result == catabox::PlayResult::FileMissing ? "Open failed: file missing"
                                                                  : "Open failed: not a playable MP3");
        return;
    }
    playing = true;
    lastPrintMs = millis();
}

void retry() {
    if (mounted || millis() - lastTryMs < 5000) {
        return;
    }
    lastTryMs = millis();
    mounted = mountSd();
    if (mounted) {
        startPlayback();
    }
}
}

void diagAudioBegin() {
    Serial.println();
    Serial.println("CataBox diagnostic: audio");
    mounted = mountSd();
    if (!mounted) {
        Serial.println("SD is required for the audio diagnostic; retrying every 5 s");
        return;
    }
    startPlayback();
}

void diagAudioLoop() {
    if (!mounted) {
        retry();
        return;
    }
    if (!playing) {
        return;
    }

    if (!player.service()) {
        Serial.printf("Playback stopped: %s at %u/%u bytes\n",
                      player.atEnd() ? "end of file" : "decode error",
                      static_cast<unsigned>(player.position()),
                      static_cast<unsigned>(player.size()));
        playing = false;
        return;
    }

    const uint32_t now = millis();
    if (now - lastPrintMs >= 1000) {
        lastPrintMs = now;
        // 128 kbps CBR is about 16,000 bytes per second.
        Serial.printf("  ~%u s\n", static_cast<unsigned>(player.position() / 16000));
    }
}

#endif
