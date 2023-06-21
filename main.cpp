#include "rtsp-constants.h"
#include "rtsp-defines.h"
#include <cstdio>
#include <cstring>
#include <iostream>
#include <memory>
#include <string>
using namespace std;

#include "rtsp-encode.h"
#include "rtsp-utils.h"

int main(int argc, char *argv[])
{
    const char   rtspUrl[] = "rtsp://192.168.1.102:554/cam/realmonitor?channel=1&subtype=0";
    RTSPUrlInfo *info = new RTSPUrlInfo;
    memset(info, 0, sizeof(RTSPUrlInfo));
    split_video_url(info, rtspUrl);

    printf("******************************************\n");
    printf("proto   : %s\n", info->proto);
    printf("auth    : %s\n", info->authorization);
    printf("hostname: %s\n", info->hostname);
    printf("port    : %d\n", info->port);
    printf("path    : %s\n", info->path);

    RTSPContext *ctx = new RTSPContext;
    memset(ctx, 0, sizeof(RTSPContext));
    sprintf(ctx->session_id, "abcdefghijklmn");
    sprintf(ctx->auth_state.realm, "Login to db1c253aeee4dfa8ee193f7a201b6f69");
    sprintf(ctx->auth_state.digest_params.nonce, "4fa166c1dc73166f9366acc51a405496");
    sprintf(ctx->auth, "admin:admin123");
    ctx->auth_state.auth_type = HTTP_AUTH_DIGEST;

    std::shared_ptr<char> ret = rtsp_method_encode(ctx, "DESCRIBE", rtspUrl, NULL);
    printf("******************************************\n");
    printf("%s", ret.get());
    printf("******************************************\n");
    return 0;
}
