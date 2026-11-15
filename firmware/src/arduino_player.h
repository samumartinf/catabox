#pragma once
#include <AudioFileSourceSD.h>
#include <AudioGeneratorMP3.h>
#include <AudioOutputI2S.h>

#include "player.h"

class ArduinoPlayer : public catabox::IPlayer {
public:
    ArduinoPlayer();

    catabox::PlayResult play(const char* path, uint32_t startByte) override;
    void stop() override;
    bool service() override;
    uint32_t position() const override;
    uint32_t size() const override;
    bool atEnd() const override;
    uint32_t fileSize(const char* path) override;

private:
    // 16 DMA buffers of 128 frames: ~46 ms of audio at 44.1 kHz, enough to
    // cover a blocking NFC poll without underrunning.
    AudioOutputI2S output_{0, AudioOutputI2S::EXTERNAL_I2S, 16};
    AudioGeneratorMP3* generator_ = nullptr;
    AudioFileSourceSD* source_ = nullptr;
    uint32_t openedSize_ = 0;
    uint32_t lastPos_ = 0;
    bool atEnd_ = false;
};
