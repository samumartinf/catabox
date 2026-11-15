#pragma once
#include <Preferences.h>

#include "bookmarks.h"

class ArduinoBookmarks : public catabox::IBookmarkStore {
public:
    uint32_t load(const char* uid) override;
    void save(const char* uid, uint32_t position) override;
    void clear(const char* uid) override;
};
