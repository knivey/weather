#include "gtest/gtest.h"

#include "../GetTime.hpp"

class GetTimeTest : public testing::Test
{
};

TEST_F(GetTimeTest, TestGetTime)
{
    // convert 12:00:00 am UTC | Saturday, January 1, 2000 to Unix time => 946684800 (Unix time)
    auto time = 946684800u;
    auto time_s = "946684800";
    ASSERT_EQ("2000-01-01 00:00:00 UTC", GetTime("%F %T %Z", time, "UTC"));
    ASSERT_EQ("1999-12-31 19:00:00 EST", GetTime("%F %T %Z", time, "EST"));
    ASSERT_EQ("1999-12-31 19:00:00 EST", GetTime("%F %T %Z", time_s, "EST"));
    ASSERT_EQ("2000-01-01 00:00:00 UTC", GetTime("%F %T %Z", time_s, "UTC"));
}
