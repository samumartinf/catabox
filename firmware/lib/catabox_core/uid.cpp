#include "uid.h"

#include <cstdio>
#include <cstring>

#include "config.h"

namespace catabox {
namespace {
constexpr char kHex[] = "0123456789ABCDEF";
}

bool formatUid(const uint8_t* bytes, uint8_t length, char out[15]) {
    if (length != 4 && length != 7) {
        return false;
    }
    for (uint8_t i = 0; i < length; ++i) {
        out[i * 2] = kHex[(bytes[i] >> 4) & 0x0F];
        out[i * 2 + 1] = kHex[bytes[i] & 0x0F];
    }
    out[length * 2] = '\0';
    return true;
}

void storyPath(const char* uid, char out[32]) {
    std::snprintf(out, 32, "%s%s.mp3", kStoriesDir, uid);
}

uint32_t resumePosition(uint32_t bookmark, uint32_t fileSize) {
    if (fileSize == 0 || bookmark < kResumeMinBytes) {
        return 0;
    }
    if (fileSize <= kResumeEndMargin || bookmark >= fileSize - kResumeEndMargin) {
        return 0;
    }
    return bookmark;
}
}
