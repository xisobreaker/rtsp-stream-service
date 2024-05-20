#include "http_auth.h"
#include "rtsp.h"
#include "rtsp_url.h"
#include <glog/logging.h>
#include <gtest/gtest.h>

TEST(TEST_RTSP, DIGEST_AUTH)
{
    const char *uri = "rtsp://192.168.1.49:554/cam/realmonitor?channel=1&subtype=0";
    RTSPContext ctx;
    memset(&ctx, 0, sizeof(RTSPContext));
    sprintf(ctx.auth, "admin:password");
    sprintf(ctx.auth_state.digest_params.nonce, "f610b28c7953f435cd7f80014c24bbac");
    sprintf(ctx.auth_state.realm, "Login to aa453f50378c821b6b22123cda43cba4");

    std::string request = "";
    /***************************** NONE *****************************/
    ctx.auth_state.auth_type = HTTP_AUTH_NONE;
    request = rtsp_method_encode(&ctx, "OPTIONS", uri, nullptr);
    EXPECT_STREQ(request.c_str(), "OPTIONS rtsp://192.168.1.49:554/cam/realmonitor?channel=1&subtype=0 RTSP/1.0\r\nCSeq: 0\r\n\r\n");

    /***************************** BASIC *****************************/
    ctx.auth_state.auth_type = HTTP_AUTH_BASIC;
    request = rtsp_method_encode(&ctx, "OPTIONS", uri, nullptr);
    EXPECT_STREQ(request.c_str(),
                 "OPTIONS rtsp://192.168.1.49:554/cam/realmonitor?channel=1&subtype=0 RTSP/1.0\r\nCSeq: 1\r\nAuthorization: Basic "
                 "YWRtaW46cGFzc3dvcmQ=\r\n\r\n");

    /***************************** DIGEST *****************************/
    ctx.auth_state.auth_type = HTTP_AUTH_DIGEST;
    request = rtsp_method_encode(&ctx, "OPTIONS", uri, nullptr);
    EXPECT_STREQ(
        request.c_str(),
        "OPTIONS rtsp://192.168.1.49:554/cam/realmonitor?channel=1&subtype=0 RTSP/1.0\r\nCSeq: 2\r\nAuthorization: Digest "
        "username=\"admin\", realm=\"Login to aa453f50378c821b6b22123cda43cba4\", nonce=\"f610b28c7953f435cd7f80014c24bbac\", "
        "uri=\"rtsp://192.168.1.49:554/cam/realmonitor?channel=1&subtype=0\", response=\"4b99786b2a560d61f23d38898418349d\"\r\n\r\n");
}

TEST(TEST_RTSP, RTSP_TRANSPORT)
{
    std::string rtspUrl = "rtsp://admin:password@192.168.1.49/cam/"
                          "realmonitor?channel=1&subtype=0";

    RTSPUrlInfo info;
    RTSPContext ctx;

    memset(&info, 0, sizeof(RTSPUrlInfo));
    memset(&ctx, 0, sizeof(RTSPContext));

    split_video_url(&info, rtspUrl.c_str());
    EXPECT_STREQ(info.proto, "rtsp");
    EXPECT_STREQ(info.authorization, "admin:password");
    EXPECT_STREQ(info.hostname, "192.168.1.49");
    EXPECT_EQ(info.port, 554);
    EXPECT_STREQ(info.path, "/cam/realmonitor?channel=1&subtype=0");
}

int main(int argc, char *argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}