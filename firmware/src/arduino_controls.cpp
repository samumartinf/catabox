#include "arduino_controls.h"

#include "config.h"

Button::Button(uint8_t pin) : pin_(pin) {}

void Button::begin() {
    pinMode(pin_, INPUT_PULLUP);
}

bool Button::pressed(uint32_t nowMs) {
    const bool raw = digitalRead(pin_) == HIGH;
    if (raw != raw_) {
        raw_ = raw;
        changedMs_ = nowMs;
    }

    if (nowMs - changedMs_ < catabox::kButtonDebounceMs || stable_ == raw_) {
        return false;
    }

    stable_ = raw;
    if (stable_ == HIGH && wasDown_) {
        wasDown_ = false;
        return true;
    }
    if (stable_ == LOW) {
        wasDown_ = true;
    }
    return false;
}
