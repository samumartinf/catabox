#pragma once
#include <cstdint>

namespace catabox {
constexpr uint32_t kAbsenceMs = 500;
constexpr uint32_t kPollMs = 100;
constexpr uint32_t kSaveIntervalMs = 10000;
constexpr uint32_t kSaveMinBytes = 32768;
constexpr uint32_t kResumeMinBytes = 8192;
constexpr uint32_t kResumeEndMargin = 8192;
constexpr uint32_t kBootDelayMs = 1000;
constexpr uint32_t kButtonDebounceMs = 40;
#ifdef CATABOX_VOLUME
constexpr float kVolume = CATABOX_VOLUME;
#else
constexpr float kVolume = 0.4f;
#endif
constexpr const char* kStoriesDir = "/stories/";
}
