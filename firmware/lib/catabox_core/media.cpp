#include "media.h"

namespace catabox {
bool looksLikeMp3(const uint8_t* data, uint8_t length) {
    if (!data || length < 2) {
        return false;
    }
    if (length >= 3 && data[0] == 'I' && data[1] == 'D' && data[2] == '3') {
        return true;
    }
    return data[0] == 0xFF && (data[1] & 0xE0) == 0xE0;
}

bool reachedEnd(uint32_t position, uint32_t size, bool sourceOpen, uint32_t margin) {
    if (!sourceOpen || size == 0 || position > size) {
        return false;
    }
    return size - position <= margin;
}
}
