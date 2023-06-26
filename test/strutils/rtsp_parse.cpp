#include "rtsp.h"
#include <cstring>
#include <iostream>
using namespace std;

int main(int argc, char *argv[])
{
    char msg[] = "WWW-Authenticate: Digest realm=\"Login to edbba66a86f75696d122e195153df0a2\", nonce=\"ba0294a902ef637b4ac19025d26d0a05\"";

    RTSPContext       *ctx = new RTSPContext;
    RTSPMessageHeader *reply = new RTSPMessageHeader;
    RTSPState         *rt = new RTSPState;

    memset(ctx, 0, sizeof(RTSPContext));
    memset(reply, 0, sizeof(RTSPMessageHeader));
    memset(rt, 0, sizeof(RTSPState));

    rtsp_parse_line(ctx, reply, msg, rt, "OPTIONS");
    return 0;
}