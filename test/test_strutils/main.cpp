#include "strutils.h"
#include <glog/logging.h>
#include <gtest/gtest.h>

TEST(TEST_STRUTILS, STRING_CUT)
{
    // 字符串裁剪
    std::string msg = "RTP/AVP/TCP";
    std::string str = string_cut_until_char(msg, "/");
    EXPECT_STREQ(str.c_str(), "RTP");
    EXPECT_STREQ(msg.c_str(), "AVP/TCP");

    // 开头匹配
    std::string transport = "Transport: RTP/AVP;unicast;client_port=18292-18293\r\n";
    std::string srcTransport = transport;
    bool        ret = string_start_and_cut(transport, "Session:");
    EXPECT_STREQ(transport.c_str(), srcTransport.c_str());
    EXPECT_EQ(ret, false);

    ret = string_start_and_cut(transport, "Transport:");
    EXPECT_STREQ(transport.c_str(), "RTP/AVP;unicast;client_port=18292-18293\r\n");
    EXPECT_EQ(ret, true);
}

int main(int argc, char *argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}