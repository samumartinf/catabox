#pragma once
#include <Arduino.h>

class Button {
public:
    explicit Button(uint8_t pin);

    void begin();
    bool pressed(uint32_t nowMs);

private:
    uint8_t pin_;
    bool raw_ = true;
    bool stable_ = true;
    bool wasDown_ = false;
    uint32_t changedMs_ = 0;
};
