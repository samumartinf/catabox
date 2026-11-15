#include <unity.h>

#include <cstring>
#include <map>
#include <string>

#include "app.h"
#include "uid.h"

void setUp(void) {}
void tearDown(void) {}

void test_format_7_byte_uid() {
    const uint8_t uid[] = {0x04, 0xA3, 0xB2, 0xC1, 0xD4, 0xE5, 0x80};
    char out[15];
    TEST_ASSERT_TRUE(catabox::formatUid(uid, 7, out));
    TEST_ASSERT_EQUAL_STRING("04A3B2C1D4E580", out);
}

void test_format_4_byte_uid() {
    const uint8_t uid[] = {0xDE, 0xAD, 0xBE, 0xEF};
    char out[15];
    TEST_ASSERT_TRUE(catabox::formatUid(uid, 4, out));
    TEST_ASSERT_EQUAL_STRING("DEADBEEF", out);
}

void test_reject_other_uid_lengths() {
    const uint8_t uid[10] = {0};
    char out[15];
    TEST_ASSERT_FALSE(catabox::formatUid(uid, 0, out));
    TEST_ASSERT_FALSE(catabox::formatUid(uid, 10, out));
}

void test_story_path() {
    char out[32];
    catabox::storyPath("04A3B2C1D4E580", out);
    TEST_ASSERT_EQUAL_STRING("/stories/04A3B2C1D4E580.mp3", out);
}

void test_resume_position_rules() {
    TEST_ASSERT_EQUAL_UINT32(0, catabox::resumePosition(0, 100000));
    TEST_ASSERT_EQUAL_UINT32(0, catabox::resumePosition(5000, 100000));
    TEST_ASSERT_EQUAL_UINT32(50000, catabox::resumePosition(50000, 100000));
    TEST_ASSERT_EQUAL_UINT32(0, catabox::resumePosition(95000, 100000));
    TEST_ASSERT_EQUAL_UINT32(0, catabox::resumePosition(50000, 0));
}

class FakePlayer : public catabox::IPlayer {
public:
    std::string lastPath;
    uint32_t lastStart = 0;
    int playCalls = 0;
    int stopCalls = 0;
    uint32_t pos = 0;
    uint32_t sz = 100000;
    bool serviceResult = true;
    bool endFlag = false;
    catabox::PlayResult nextResult = catabox::PlayResult::Ok;

    catabox::PlayResult play(const char* path, uint32_t start) override {
        lastPath = path;
        lastStart = start;
        playCalls++;
        pos = start;
        endFlag = false;
        return nextResult;
    }
    void stop() override { stopCalls++; }
    bool service() override { return serviceResult; }
    uint32_t position() const override { return pos; }
    uint32_t size() const override { return sz; }
    bool atEnd() const override { return endFlag; }
    uint32_t fileSize(const char*) override { return sz; }
};

class FakeBookmarks : public catabox::IBookmarkStore {
public:
    std::map<std::string, uint32_t> values;
    uint32_t load(const char* uid) override {
        auto it = values.find(uid);
        return it == values.end() ? 0 : it->second;
    }
    void save(const char* uid, uint32_t pos) override { values[uid] = pos; }
    void clear(const char* uid) override { values.erase(uid); }
};

static const char* kUid = "04A3B2C1D4E580";

void test_tag_plays_story_from_start() {
    FakePlayer player;
    FakeBookmarks marks;
    catabox::App app(player, marks);
    app.onScan(kUid, 0);
    TEST_ASSERT_EQUAL_STRING("/stories/04A3B2C1D4E580.mp3", player.lastPath.c_str());
    TEST_ASSERT_EQUAL_UINT32(0, player.lastStart);
    TEST_ASSERT_TRUE(app.state() == catabox::AppState::Playing);
    TEST_ASSERT_TRUE(app.tagPresent());
}

void test_repeated_scans_do_not_restart() {
    FakePlayer player;
    FakeBookmarks marks;
    catabox::App app(player, marks);
    app.onScan(kUid, 0);
    app.onScan(kUid, 100);
    app.onScan(kUid, 200);
    TEST_ASSERT_EQUAL_INT(1, player.playCalls);
}

void test_short_dropout_is_ignored() {
    FakePlayer player;
    FakeBookmarks marks;
    catabox::App app(player, marks);
    app.onScan(kUid, 0);
    app.onScan(nullptr, 100);
    app.onScan(kUid, 200);
    TEST_ASSERT_EQUAL_INT(1, player.playCalls);
    TEST_ASSERT_EQUAL_INT(0, player.stopCalls);
    TEST_ASSERT_TRUE(app.state() == catabox::AppState::Playing);
}

void test_lift_pauses_and_saves() {
    FakePlayer player;
    FakeBookmarks marks;
    catabox::App app(player, marks);
    app.onScan(kUid, 0);
    player.pos = 40000;
    app.onScan(nullptr, 600);
    TEST_ASSERT_EQUAL_INT(1, player.stopCalls);
    TEST_ASSERT_EQUAL_UINT32(40000, marks.values[kUid]);
    TEST_ASSERT_FALSE(app.tagPresent());
    TEST_ASSERT_TRUE(app.state() == catabox::AppState::Paused);
}

void test_return_resumes_at_saved_position() {
    FakePlayer player;
    FakeBookmarks marks;
    catabox::App app(player, marks);
    app.onScan(kUid, 0);
    player.pos = 40000;
    app.onScan(nullptr, 600);
    app.onScan(kUid, 700);
    TEST_ASSERT_EQUAL_UINT32(40000, player.lastStart);
    TEST_ASSERT_TRUE(app.state() == catabox::AppState::Playing);
}

void test_user_pause_survives_lift_and_return() {
    FakePlayer player;
    FakeBookmarks marks;
    catabox::App app(player, marks);
    app.onScan(kUid, 0);
    player.pos = 30000;
    app.onPauseButton(100);
    TEST_ASSERT_TRUE(app.userPaused());
    app.onScan(nullptr, 700);
    app.onScan(kUid, 800);
    TEST_ASSERT_EQUAL_INT(1, player.playCalls);
    TEST_ASSERT_TRUE(app.userPaused());
    app.onPauseButton(900);
    TEST_ASSERT_EQUAL_UINT32(30000, player.lastStart);
    TEST_ASSERT_TRUE(app.state() == catabox::AppState::Playing);
}

void test_different_tag_while_current_still_seen_is_ignored() {
    FakePlayer player;
    FakeBookmarks marks;
    catabox::App app(player, marks);
    app.onScan(kUid, 0);
    app.onScan("DEADBEEF", 100);
    TEST_ASSERT_EQUAL_INT(1, player.playCalls);
    TEST_ASSERT_EQUAL_STRING(kUid, app.loadedUid());
}

void test_different_tag_after_absence_swaps_story() {
    FakePlayer player;
    FakeBookmarks marks;
    catabox::App app(player, marks);
    app.onScan(kUid, 0);
    player.pos = 25000;
    app.onScan("DEADBEEF", 600);
    TEST_ASSERT_EQUAL_UINT32(25000, marks.values[kUid]);
    TEST_ASSERT_EQUAL_STRING("/stories/DEADBEEF.mp3", player.lastPath.c_str());
    TEST_ASSERT_EQUAL_UINT32(0, player.lastStart);
}

void test_restart_needs_tag_present() {
    FakePlayer player;
    FakeBookmarks marks;
    catabox::App app(player, marks);
    app.onScan(kUid, 0);
    app.onScan(nullptr, 600);
    int calls = player.playCalls;
    app.onRestartButton(700);
    TEST_ASSERT_EQUAL_INT(calls, player.playCalls);
}

void test_restart_rewinds_and_clears_bookmark() {
    FakePlayer player;
    FakeBookmarks marks;
    catabox::App app(player, marks);
    app.onScan(kUid, 0);
    marks.values[kUid] = 50000;
    app.onRestartButton(100);
    TEST_ASSERT_EQUAL_UINT32(0, player.lastStart);
    TEST_ASSERT_EQUAL_UINT32(0, static_cast<uint32_t>(marks.values.count(kUid)));
    TEST_ASSERT_TRUE(app.state() == catabox::AppState::Playing);
}

void test_finish_clears_bookmark_and_does_not_replay() {
    FakePlayer player;
    FakeBookmarks marks;
    catabox::App app(player, marks);
    app.onScan(kUid, 0);
    marks.values[kUid] = 60000;
    player.pos = 100000;
    player.endFlag = true;
    player.serviceResult = false;
    app.tick(100);
    TEST_ASSERT_EQUAL_UINT32(0, static_cast<uint32_t>(marks.values.count(kUid)));
    int calls = player.playCalls;
    app.onScan(kUid, 200);
    app.tick(300);
    app.onScan(kUid, 400);
    TEST_ASSERT_EQUAL_INT(calls, player.playCalls);
}

void test_finished_tag_replays_after_removal() {
    FakePlayer player;
    FakeBookmarks marks;
    catabox::App app(player, marks);
    app.onScan(kUid, 0);
    player.pos = 100000;
    player.endFlag = true;
    player.serviceResult = false;
    app.tick(100);
    app.onScan(nullptr, 700);
    app.onScan(kUid, 800);
    TEST_ASSERT_EQUAL_UINT32(0, player.lastStart);
    TEST_ASSERT_TRUE(app.state() == catabox::AppState::Playing);
}

void test_decode_error_keeps_bookmark_and_latches() {
    FakePlayer player;
    FakeBookmarks marks;
    catabox::App app(player, marks);
    app.onScan(kUid, 0);
    player.pos = 30000;
    player.endFlag = false;
    player.serviceResult = false;
    app.tick(100);
    TEST_ASSERT_EQUAL_UINT32(30000, marks.values[kUid]);
    int calls = player.playCalls;
    app.onScan(kUid, 200);
    TEST_ASSERT_EQUAL_INT(calls, player.playCalls);
}

void test_missing_file_is_handled_and_retried_after_removal() {
    FakePlayer player;
    FakeBookmarks marks;
    catabox::App app(player, marks);
    player.nextResult = catabox::PlayResult::FileMissing;
    app.onScan(kUid, 0);
    TEST_ASSERT_TRUE(app.state() == catabox::AppState::Idle);
    TEST_ASSERT_TRUE(app.lastError()[0] != '\0');
    int calls = player.playCalls;
    app.onScan(kUid, 100);
    TEST_ASSERT_EQUAL_INT(calls, player.playCalls);
    app.onScan(nullptr, 700);
    player.nextResult = catabox::PlayResult::Ok;
    app.onScan(kUid, 800);
    TEST_ASSERT_EQUAL_INT(calls + 1, player.playCalls);
}

void test_audio_open_error_is_handled_and_latched() {
    FakePlayer player;
    FakeBookmarks marks;
    catabox::App app(player, marks);
    player.nextResult = catabox::PlayResult::Error;
    app.onScan(kUid, 0);
    TEST_ASSERT_TRUE(app.state() == catabox::AppState::Idle);
    TEST_ASSERT_TRUE(app.lastError()[0] != '\0');
    int calls = player.playCalls;
    app.onScan(kUid, 100);
    TEST_ASSERT_EQUAL_INT(calls, player.playCalls);
    app.onScan(nullptr, 700);
    player.nextResult = catabox::PlayResult::Ok;
    app.onScan(kUid, 800);
    TEST_ASSERT_EQUAL_INT(calls + 1, player.playCalls);
}

void test_button_without_tag_does_nothing() {
    FakePlayer player;
    FakeBookmarks marks;
    catabox::App app(player, marks);
    app.onPauseButton(0);
    app.onRestartButton(0);
    TEST_ASSERT_EQUAL_INT(0, player.playCalls);
}

void test_short_tag_bounce_does_not_swap() {
    FakePlayer player;
    FakeBookmarks marks;
    catabox::App app(player, marks);
    app.onScan(kUid, 0);
    app.onScan("DEADBEEF", 300);
    app.onScan(kUid, 400);
    TEST_ASSERT_EQUAL_STRING(kUid, app.loadedUid());
    TEST_ASSERT_EQUAL_INT(1, player.playCalls);
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_format_7_byte_uid);
    RUN_TEST(test_format_4_byte_uid);
    RUN_TEST(test_reject_other_uid_lengths);
    RUN_TEST(test_story_path);
    RUN_TEST(test_resume_position_rules);
    RUN_TEST(test_tag_plays_story_from_start);
    RUN_TEST(test_repeated_scans_do_not_restart);
    RUN_TEST(test_short_dropout_is_ignored);
    RUN_TEST(test_lift_pauses_and_saves);
    RUN_TEST(test_return_resumes_at_saved_position);
    RUN_TEST(test_user_pause_survives_lift_and_return);
    RUN_TEST(test_different_tag_while_current_still_seen_is_ignored);
    RUN_TEST(test_different_tag_after_absence_swaps_story);
    RUN_TEST(test_restart_needs_tag_present);
    RUN_TEST(test_restart_rewinds_and_clears_bookmark);
    RUN_TEST(test_finish_clears_bookmark_and_does_not_replay);
    RUN_TEST(test_finished_tag_replays_after_removal);
    RUN_TEST(test_decode_error_keeps_bookmark_and_latches);
    RUN_TEST(test_missing_file_is_handled_and_retried_after_removal);
    RUN_TEST(test_audio_open_error_is_handled_and_latched);
    RUN_TEST(test_button_without_tag_does_nothing);
    RUN_TEST(test_short_tag_bounce_does_not_swap);
    return UNITY_END();
}
