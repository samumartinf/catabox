#include "arduino_nfc.h"

#include <Wire.h>

#include "uid.h"

bool ArduinoNfc::begin() {
    Wire.begin(21, 22);
    if (!nfc_.begin()) {
        return false;
    }
    return nfc_.SAMConfig();
}

bool ArduinoNfc::readUid(char out[15]) {
    uint8_t uid[10] = {};
    uint8_t length = 0;
    if (!nfc_.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &length, 20)) {
        return false;
    }
    return catabox::formatUid(uid, length, out);
}

uint32_t ArduinoNfc::firmwareVersion() {
    return nfc_.getFirmwareVersion();
}
