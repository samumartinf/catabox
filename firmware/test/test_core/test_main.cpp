#include <unity.h>
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

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_format_7_byte_uid);
    RUN_TEST(test_format_4_byte_uid);
    RUN_TEST(test_reject_other_uid_lengths);
    RUN_TEST(test_story_path);
    RUN_TEST(test_resume_position_rules);
    return UNITY_END();
}
