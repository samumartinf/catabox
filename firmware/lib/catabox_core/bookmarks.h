#pragma once
#include <cstdint>

namespace catabox {
class IBookmarkStore {
public:
    virtual ~IBookmarkStore() = default;
    virtual uint32_t load(const char* uid) = 0;
    virtual void save(const char* uid, uint32_t position) = 0;
    virtual void clear(const char* uid) = 0;
};
}
