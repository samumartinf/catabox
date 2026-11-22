#pragma once
#include <cstdint>

namespace catabox {
// True when the first bytes look like an MP3 (ID3v2 tag or MPEG frame sync).
bool looksLikeMp3(const uint8_t* data, uint8_t length);

// True only for a clean end of a non-empty, still-open file.
bool reachedEnd(uint32_t position, uint32_t size, bool sourceOpen, uint32_t margin = 4096);

// True when the filename ends in .mp3 (case-insensitive).
bool hasMp3Extension(const char* name);
}
