#include "http_auth.h"
#include "rtsp.h"
#include "rtsp_url.h"
#include "sdp.h"
#include <glog/logging.h>
#include <gtest/gtest.h>

TEST(TEST_RTSP_PARSE, RTSP_PARSE)
{
    RTSPContext ctx;
    RTSPMessage reply;

    memset(&ctx, 0, sizeof(RTSPContext));
    memset(&reply, 0, sizeof(RTSPMessage));

    const char *wwwAuth =
        "WWW-Authenticate: Digest realm=\"Login to edbba66a86f75696d122e195153df0a2\", nonce=\"ba0294a902ef637b4ac19025d26d0a05\"";
    rtsp_parse_line(&ctx, &reply, wwwAuth, "OPTIONS");

    EXPECT_EQ(ctx.auth_state.auth_type, HTTP_AUTH_DIGEST);
    EXPECT_STREQ(ctx.auth_state.realm, "Login to edbba66a86f75696d122e195153df0a2");
    EXPECT_STREQ(ctx.auth_state.digest_params.nonce, "ba0294a902ef637b4ac19025d26d0a05");

    const char *session = "Session: 2936719851115;timeout=60";
    rtsp_parse_line(&ctx, &reply, session, "SETUP");
    EXPECT_STREQ(reply.session_id, "2936719851115");
    EXPECT_EQ(reply.timeout, 60);

    const char *contentLen = "Content-Length: 465";
    rtsp_parse_line(&ctx, &reply, contentLen, "DESCRIBE");
    EXPECT_EQ(reply.content_length, 465);

    const char *transport = "Transport: RTP/AVP/TCP;unicast;client_port=18292-18293\r\n"
                            "RTP/AVP;unicast;client_port=34234-34235;server_port=20908-20909;ssrc=3E0C69CB\r\n";
    rtsp_parse_line(&ctx, &reply, transport, "DESCRIBE");
    EXPECT_EQ(reply.nb_transports, 2);
    EXPECT_EQ(reply.transports[0].client_port_min, 18292);
    EXPECT_EQ(reply.transports[0].client_port_max, 18293);
    EXPECT_EQ(reply.transports[1].client_port_min, 34234);
    EXPECT_EQ(reply.transports[1].client_port_max, 34235);
    EXPECT_EQ(reply.transports[1].server_port_min, 20908);
    EXPECT_EQ(reply.transports[1].server_port_max, 20909);

    const char *range = "Range: npt=0.000-";
    rtsp_parse_line(&ctx, &reply, range, "PLAY");
}

TEST(TEST_RTSP_PARSE, SDP_PARSE)
{
    std::string msg =
        "v=0\r\n"
        "o=- 2256453820 2256453820 IN IP4 0.0.0.0\r\n"
        "s=Media Server\r\n"
        "c=IN IP4 0.0.0.0\r\n"
        "t=0 0\r\n"
        "a=control:*\r\n"
        "a=packetization-supported:DH\r\n"
        "a=rtppayload-supported:DH\r\n"
        "a=range:npt=now-\r\n"
        "m=video 0 RTP/AVP 96\r\n"
        "a=control:trackID=0\r\n"
        "a=framerate:10.000000\r\n"
        "a=rtpmap:96 H264/90000\r\n"
        "a=fmtp:96 packetization-mode=1;profile-level-id=64001F;sprop-parameter-sets=Z2QAH6wbGoBQBb/m4CAgKAAAH0AAAnEHQwBj4AAas/"
        "XeXGhgDHwAA1Z+u8uFAA==,aO44MAA=\r\n"
        "a=recvonly\r\n";
    struct SDPPayload *sdp = sdp_parser(msg.c_str());
    sdp_print(sdp);

    std::string sdp_message = sdp_format(sdp);
    printf("%s", sdp_message.c_str());
    printf("\n");
}

int main(int argc, char *argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}