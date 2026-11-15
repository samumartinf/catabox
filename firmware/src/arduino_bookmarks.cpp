#include "arduino_bookmarks.h"

namespace {
constexpr const char* kNamespace = "catabox";
}

uint32_t ArduinoBookmarks::load(const char* uid) {
    Preferences prefs;
    if (!prefs.begin(kNamespace, true)) {
        return 0;
    }
    const uint32_t position = prefs.getUInt(uid, 0);
    prefs.end();
    return position;
}

void ArduinoBookmarks::save(const char* uid, uint32_t position) {
    Preferences prefs;
    if (!prefs.begin(kNamespace, false)) {
        return;
    }
    prefs.putUInt(uid, position);
    prefs.end();
}

void ArduinoBookmarks::clear(const char* uid) {
    Preferences prefs;
    if (!prefs.begin(kNamespace, false)) {
        return;
    }
    prefs.remove(uid);
    prefs.end();
}
