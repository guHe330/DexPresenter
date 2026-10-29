/**
 * DexPresenter - a free and open source Windows presentation utility with drawing tools
 * Copyright (C) 2026 Gunter Heiss
 *
 * The DexPresenter project is hosted on GitHub: https://github.com/guHe330/DexPresenter
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

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
