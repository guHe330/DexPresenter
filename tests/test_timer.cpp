#include <gtest/gtest.h>
#include "Config.h"

TEST(FormatTime, TenMinutes) {
    EXPECT_EQ(FormatTime(600), L"10:00");
}

TEST(FormatTime, OneHourOneMinuteOneSecond) {
    EXPECT_EQ(FormatTime(3661), L"1:01:01");
}

TEST(FormatTime, Zero) {
    EXPECT_EQ(FormatTime(0), L"00:00");
}

TEST(FormatTime, FiftyNineSeconds) {
    EXPECT_EQ(FormatTime(59), L"00:59");
}

TEST(FormatTime, OneSecond) {
    EXPECT_EQ(FormatTime(1), L"00:01");
}

TEST(FormatTime, ExactlyOneHour) {
    EXPECT_EQ(FormatTime(3600), L"1:00:00");
}

TEST(FormatTime, NegativeClampedToZero) {
    EXPECT_EQ(FormatTime(-5), L"00:00");
}
