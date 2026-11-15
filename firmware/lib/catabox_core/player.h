#pragma once
#include <cstdint>

namespace catabox {
enum class PlayResult { Ok, FileMissing, Error };

class IPlayer {
public:
    virtual ~IPlayer() = default;
    virtual PlayResult play(const char* path, uint32_t startByte) = 0;
    virtual void stop() = 0;
    virtual bool service() = 0;
    virtual uint32_t position() const = 0;
    virtual uint32_t size() const = 0;
    virtual bool atEnd() const = 0;
    virtual uint32_t fileSize(const char* path) = 0;
};
}
