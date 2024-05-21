#include "strutils.h"
#include <glog/logging.h>
#include <gtest/gtest.h>

TEST(TEST_STRUTILS, STRING_CUT)
{
    std::string msg = "RTP/AVP/TCP";
    std::string str = string_cut_until_char(msg, '/');
    EXPECT_STREQ(str.c_str(), "RTP");
    EXPECT_STREQ(msg.c_str(), "AVP/TCP");
}

int main(int argc, char *argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}