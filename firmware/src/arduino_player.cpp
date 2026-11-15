#include "arduino_player.h"

#include <SD.h>

#include <cstdio>

#include "config.h"
#include "media.h"

ArduinoPlayer::ArduinoPlayer() {
    output_.SetPinout(27, 26, 25);
    output_.SetGain(catabox::kVolume);
}

catabox::PlayResult ArduinoPlayer::play(const char* path, uint32_t startByte) {
    stop();
    atEnd_ = false;
    openedSize_ = 0;
    lastPos_ = startByte;

    source_ = new AudioFileSourceSD(path);
    if (!source_->isOpen()) {
        delete source_;
        source_ = nullptr;
        return catabox::PlayResult::FileMissing;
    }

    openedSize_ = source_->getSize();
    if (openedSize_ == 0) {
        stop();
        return catabox::PlayResult::Error;
    }

    uint8_t header[3] = {};
    const uint32_t got = source_->read(header, sizeof(header));
    if (got != sizeof(header) || !catabox::looksLikeMp3(header, sizeof(header))) {
        stop();
        return catabox::PlayResult::Error;
    }

    if (!source_->seek(static_cast<int32_t>(startByte), SEEK_SET)) {
        stop();
        return catabox::PlayResult::Error;
    }

    generator_ = new AudioGeneratorMP3();
    if (!generator_->begin(source_, &output_)) {
        stop();
        return catabox::PlayResult::Error;
    }
    return catabox::PlayResult::Ok;
}

void ArduinoPlayer::stop() {
    if (generator_) {
        generator_->stop();
        delete generator_;
        generator_ = nullptr;
    }
    if (source_) {
        source_->close();
        delete source_;
        source_ = nullptr;
    }
}

bool ArduinoPlayer::service() {
    if (!generator_ || !generator_->isRunning()) {
        return false;
    }

    const bool running = generator_->loop();
    if (running) {
        if (source_ && source_->isOpen()) {
            lastPos_ = source_->getPos();
        }
    } else {
        atEnd_ = catabox::reachedEnd(position(), openedSize_, source_ && source_->isOpen());
    }
    return running;
}

uint32_t ArduinoPlayer::position() const {
    if (source_ && source_->isOpen()) {
        return source_->getPos();
    }
    return lastPos_;
}

uint32_t ArduinoPlayer::size() const {
    return openedSize_;
}

bool ArduinoPlayer::atEnd() const {
    return atEnd_;
}

uint32_t ArduinoPlayer::fileSize(const char* path) {
    File file = SD.open(path);
    if (!file) {
        return 0;
    }
    const uint32_t bytes = file.size();
    file.close();
    return bytes;
}
