#pragma once
#include <cstdint>

#include "bookmarks.h"
#include "player.h"

namespace catabox {
enum class AppState { Idle, Playing, Paused };
using ErrorCallback = void (*)(const char* message);

class App {
public:
    App(IPlayer& player, IBookmarkStore& bookmarks, ErrorCallback onError = nullptr);

    void onScan(const char* uidOrNull, uint32_t nowMs);
    void onPauseButton(uint32_t nowMs);
    void onRestartButton(uint32_t nowMs);
    void tick(uint32_t nowMs);

    AppState state() const;
    bool tagPresent() const;
    bool userPaused() const;
    const char* loadedUid() const;
    const char* lastError() const;

private:
    void handleAbsent(uint32_t nowMs);
    void selectTag(const char* uid, uint32_t nowMs);
    void startStory(const char* uid, uint32_t nowMs);
    void playAt(uint32_t startByte, uint32_t nowMs);
    void pausePlayback();
    void saveBookmark();
    bool latched(const char* uid) const;
    void setError(const char* message);

    IPlayer& player_;
    IBookmarkStore& bookmarks_;
    ErrorCallback onError_;

    char selected_[15] = {};
    char loaded_[15] = {};
    char finished_[15] = {};
    char failed_[15] = {};
    char path_[32] = {};
    char lastError_[64] = {};

    uint32_t lastSeenMs_ = 0;
    uint32_t lastSavedMs_ = 0;
    uint32_t lastSavedPos_ = 0;
    uint32_t pausedPos_ = 0;

    bool tagPresent_ = false;
    bool userPaused_ = false;
    bool playing_ = false;
};
}
