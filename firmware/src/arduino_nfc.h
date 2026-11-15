#pragma once
#include <Adafruit_PN532.h>

class ArduinoNfc {
public:
    bool begin();
    bool readUid(char out[15]);

private:
    Adafruit_PN532 nfc_{255, 255};
};
