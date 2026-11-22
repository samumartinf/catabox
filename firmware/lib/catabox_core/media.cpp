#include "media.h"

#include <cctype>
#include <cstring>

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

bool hasMp3Extension(const char* name) {
    if (!name) {
        return false;
    }
    const size_t length = std::strlen(name);
    if (length < 4) {
        return false;
    }
    const char* ext = name + length - 4;
    return ext[0] == '.' && std::tolower(static_cast<unsigned char>(ext[1])) == 'm' &&
           std::tolower(static_cast<unsigned char>(ext[2])) == 'p' &&
           std::tolower(static_cast<unsigned char>(ext[3])) == '3';
}
}

