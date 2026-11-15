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
    AudioOutputI2S output_;
    AudioGeneratorMP3* generator_ = nullptr;
    AudioFileSourceSD* source_ = nullptr;
    bool atEnd_ = false;
};
