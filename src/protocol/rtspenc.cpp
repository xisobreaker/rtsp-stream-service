#include "rtspenc.h"

#include "base64.h"
#include "memutils.h"
#include "rtsputils.h"
#include "strutils.h"
#include "urlcodec.h"
#include <math.h>
#include <memory>

using namespace std;

#ifndef MAX_RTSP_SIZE
#define MAX_RTSP_SIZE 4096
#endif

std::shared_ptr<char> rtsp_method_encode(RTSPContext *ctx, const char *method, const char *uri, const char *headers)
{
    std::shared_ptr<char> buf(new char[MAX_RTSP_SIZE], std::default_delete<char[]>());
    snprintf(buf.get(), MAX_RTSP_SIZE, "%s %s RTSP/1.0\r\n", method, uri);
    if (headers) {
        string_lcat(buf.get(), headers, MAX_RTSP_SIZE);
    }
    snprint_lcatf(buf.get(), MAX_RTSP_SIZE, "CSeq: %d\r\n", ctx->seq++);
    if (ctx->user_agent) {
        snprint_lcatf(buf.get(), MAX_RTSP_SIZE, "User-Agent: %s\r\n", ctx->user_agent);
    }
    if (ctx->session_id[0] != '\0') {
        snprint_lcatf(buf.get(), MAX_RTSP_SIZE, "Session: %s\r\n", ctx->session_id);
    }
    if (ctx->auth[0] != '\0') {
        std::shared_ptr<char> auth = http_auth_create_response(&ctx->auth_state, ctx->auth, uri, method);
        if (auth) {
            string_lcat(buf.get(), auth.get(), MAX_RTSP_SIZE);
        }
    }
    string_lcat(buf.get(), "\r\n", MAX_RTSP_SIZE);

    if (ctx->control_transport == RTSP_MODE_TUNNEL) {
        int                   base64MaxSize = ceil(MAX_RTSP_SIZE / 3.0) * 4 + 1;
        std::shared_ptr<char> base64Buf(new char[base64MaxSize], std::default_delete<char[]>());
        base64_encode(buf.get(), strlen(buf.get()), base64Buf.get(), base64MaxSize);
        buf = std::move(base64Buf);
    }
    return buf;
}
