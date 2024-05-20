#include "rtsp.h"
#include "rtsp_url.h"
#include <glog/logging.h>
#include <gtest/gtest.h>

TEST(TEST_RTSP, RTSP_TRANSPORT)
{
    std::string rtspUrl = "rtsp://admin:password@192.168.2.13:554/cam/"
                          "realmonitor?channel=1&subtype=0";

    RTSPUrlInfo info;
    RTSPContext ctx;

    memset(&info, 0, sizeof(RTSPUrlInfo));
    memset(&ctx, 0, sizeof(RTSPContext));

    split_video_url(&info, rtspUrl.c_str());
    EXPECT_STREQ(info.proto, "rtsp");
    EXPECT_STREQ(info.authorization, "admin:password");
    EXPECT_STREQ(info.hostname, "192.168.2.13");
    EXPECT_EQ(info.port, 554);
    EXPECT_STREQ(info.path, "/cam/realmonitor?channel=1&subtype=0");

    std::string options_req = rtsp_method_encode(&ctx, "OPTIONS", rtspUrl.c_str(), nullptr);
    LOG(INFO) << "[" << options_req << "]";
}

int main(int argc, char *argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}