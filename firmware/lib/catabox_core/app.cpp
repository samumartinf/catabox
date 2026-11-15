#include "app.h"

#include <cstring>

#include "config.h"
#include "uid.h"

namespace catabox {
namespace {
bool same(const char* a, const char* b) {
    return std::strcmp(a, b) == 0;
}

void copyUid(char (&dst)[15], const char* src) {
    std::strncpy(dst, src, 14);
    dst[14] = '\0';
}
}

App::App(IPlayer& player, IBookmarkStore& bookmarks, ErrorCallback onError)
    : player_(player), bookmarks_(bookmarks), onError_(onError) {}

void App::onScan(const char* uidOrNull, uint32_t nowMs) {
    if (!uidOrNull || !uidOrNull[0]) {
        handleAbsent(nowMs);
        return;
    }

    if (!selected_[0]) {
        selectTag(uidOrNull, nowMs);
        return;
    }

    if (same(uidOrNull, selected_)) {
        lastSeenMs_ = nowMs;
        if (!tagPresent_) {
            tagPresent_ = true;
            if (!userPaused_) {
                startStory(selected_, nowMs);
            }
        }
        return;
    }

    if (nowMs - lastSeenMs_ < kAbsenceMs) {
        return;
    }

    handleAbsent(nowMs);
    selectTag(uidOrNull, nowMs);
}

void App::onPauseButton(uint32_t nowMs) {
    if (!tagPresent_ || !loaded_[0] || latched(loaded_)) {
        return;
    }

    if (!userPaused_) {
        userPaused_ = true;
        if (playing_) {
            pausePlayback();
        }
        return;
    }

    userPaused_ = false;
    if (!playing_) {
        playAt(pausedPos_, nowMs);
    }
}

void App::onRestartButton(uint32_t nowMs) {
    (void)nowMs;
}

void App::tick(uint32_t nowMs) {
    if (!playing_) {
        return;
    }

    if (!player_.service()) {
        pausedPos_ = player_.position();
        player_.stop();
        playing_ = false;
        return;
    }

    const uint32_t pos = player_.position();
    if (nowMs - lastSavedMs_ >= kSaveIntervalMs && pos >= lastSavedPos_ + kSaveMinBytes) {
        bookmarks_.save(loaded_, pos);
        lastSavedMs_ = nowMs;
        lastSavedPos_ = pos;
    }
}

AppState App::state() const {
    if (playing_) {
        return AppState::Playing;
    }
    if (loaded_[0] && !latched(loaded_)) {
        return AppState::Paused;
    }
    return AppState::Idle;
}

bool App::tagPresent() const { return tagPresent_; }
bool App::userPaused() const { return userPaused_; }
const char* App::loadedUid() const { return loaded_; }
const char* App::lastError() const { return lastError_; }

void App::handleAbsent(uint32_t nowMs) {
    if (!selected_[0]) {
        return;
    }
    if (nowMs - lastSeenMs_ < kAbsenceMs) {
        return;
    }

    selected_[0] = '\0';
    if (tagPresent_) {
        tagPresent_ = false;
        if (playing_) {
            pausePlayback();
        }
    }

    if (finished_[0] && same(finished_, loaded_)) {
        finished_[0] = '\0';
    }
    if (failed_[0] && same(failed_, loaded_)) {
        failed_[0] = '\0';
    }
}

void App::selectTag(const char* uid, uint32_t nowMs) {
    copyUid(selected_, uid);
    lastSeenMs_ = nowMs;
    tagPresent_ = true;

    if (latched(uid)) {
        return;
    }
    if (same(uid, loaded_) && userPaused_) {
        return;
    }

    startStory(uid, nowMs);
}

void App::startStory(const char* uid, uint32_t nowMs) {
    if (!same(uid, loaded_)) {
        if (playing_) {
            pausePlayback();
        } else if (loaded_[0]) {
            saveBookmark();
        }
        copyUid(loaded_, uid);
        userPaused_ = false;
        storyPath(loaded_, path_);
    }

    const uint32_t start = resumePosition(bookmarks_.load(loaded_), player_.fileSize(path_));
    playAt(start, nowMs);
}

void App::playAt(uint32_t startByte, uint32_t nowMs) {
    pausedPos_ = startByte;
    const PlayResult result = player_.play(path_, startByte);
    if (result == PlayResult::Ok) {
        playing_ = true;
        lastSavedMs_ = nowMs;
        lastSavedPos_ = startByte;
        return;
    }

    playing_ = false;
    copyUid(failed_, loaded_);
    setError(result == PlayResult::FileMissing ? "story file missing" : "audio open failed");
}

void App::pausePlayback() {
    pausedPos_ = player_.position();
    player_.stop();
    playing_ = false;
    saveBookmark();
}

void App::saveBookmark() {
    if (loaded_[0]) {
        bookmarks_.save(loaded_, pausedPos_);
    }
}

bool App::latched(const char* uid) const {
    return (finished_[0] && same(finished_, uid)) || (failed_[0] && same(failed_, uid));
}

void App::setError(const char* message) {
    std::strncpy(lastError_, message, sizeof(lastError_) - 1);
    lastError_[sizeof(lastError_) - 1] = '\0';
    if (onError_) {
        onError_(lastError_);
    }
}
}
