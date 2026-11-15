#pragma once
#include <cstdint>

namespace catabox {
bool formatUid(const uint8_t* bytes, uint8_t length, char out[15]);
void storyPath(const char* uid, char out[32]);
uint32_t resumePosition(uint32_t bookmark, uint32_t fileSize);
}
