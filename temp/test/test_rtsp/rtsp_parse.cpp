#include "rtsp.h"
#include "rtspdefs.h"
#include "rtsputils.h"
#include <cstring>
#include <glog/logging.h>
#include <gtest/gtest.h>
#include <iostream>

using namespace std;

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

TEST(TEST_RTSP, PARSE_LINE)
{
    RTSPContext       *ctx = new RTSPContext;
    RTSPMessageHeader *reply = new RTSPMessageHeader;
    RTSPState         *state = new RTSPState;

    memset(ctx, 0, sizeof(RTSPContext));
    memset(reply, 0, sizeof(RTSPMessageHeader));
    memset(state, 0, sizeof(RTSPState));

    char wwwAuth[] = "WWW-Authenticate: Digest realm=\"Login to "
                     "edbba66a86f75696d122e195153df0a2\", "
                     "nonce=\"ba0294a902ef637b4ac19025d26d0a05\"";
    rtsp_parse_line(ctx, reply, wwwAuth, state, "OPTIONS");

    char session[] = "Session: 2936719851115;timeout=60";
    rtsp_parse_line(ctx, reply, session, state, "SETUP");

    char contentLen[] = "Content-Length: 465";
    rtsp_parse_line(ctx, reply, contentLen, state, "DESCRIBE");

    char transport[] = "Transport: RTP/AVP;unicast;client_port=18292-18293";
    rtsp_parse_line(ctx, reply, transport, state, "DESCRIBE");

    char range[] = "Range: npt=3.51-324.39";
    rtsp_parse_line(ctx, reply, range, state, "PLAY");

    LOG(INFO) << ": " << reply->range_start << ", " << reply->range_end;
}

int main(int argc, char *argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

#if 0
void test_rtsp_parse_line()
{
    RTSPContext       *ctx = new RTSPContext;
    RTSPMessageHeader *reply = new RTSPMessageHeader;
    RTSPState         *state = new RTSPState;

    memset(ctx, 0, sizeof(RTSPContext));
    memset(reply, 0, sizeof(RTSPMessageHeader));
    memset(state, 0, sizeof(RTSPState));

    char wwwAuth[] =
        "WWW-Authenticate: Digest realm=\"Login to edbba66a86f75696d122e195153df0a2\", nonce=\"ba0294a902ef637b4ac19025d26d0a05\"";
    rtsp_parse_line(ctx, reply, wwwAuth, state, "OPTIONS");

    char session[] = "Session: 2936719851115;timeout=60";
    rtsp_parse_line(ctx, reply, session, state, "SETUP");

    char contentLen[] = "Content-Length: 465";
    rtsp_parse_line(ctx, reply, contentLen, state, "DESCRIBE");

    char transport[] = "Transport: RTP/AVP;unicast;client_port=18292-18293";
    rtsp_parse_line(ctx, reply, transport, state, "DESCRIBE");

    char range[] = "Range: npt=3.51-324.39";
    rtsp_parse_line(ctx, reply, range, state, "PLAY");
}
#endif