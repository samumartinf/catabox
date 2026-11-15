#include "arduino_player.h"

#include <SD.h>

#include "config.h"

ArduinoPlayer::ArduinoPlayer() {
    output_.SetPinout(27, 26, 25);
    output_.SetGain(catabox::kVolume);
}

catabox::PlayResult ArduinoPlayer::play(const char* path, uint32_t startByte) {
    stop();
    atEnd_ = false;

    source_ = new AudioFileSourceSD(path);
    if (!source_->isOpen()) {
        delete source_;
        source_ = nullptr;
        return catabox::PlayResult::FileMissing;
    }

    if (startByte > 0) {
        source_->seek(startByte, SEEK_SET);
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
    if (!running) {
        atEnd_ = position() + 4096 >= size();
    }
    return running;
}

uint32_t ArduinoPlayer::position() const {
    return source_ ? source_->getPos() : 0;
}

uint32_t ArduinoPlayer::size() const {
    return source_ ? source_->getSize() : 0;
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
