#include "rtsp.h"
#include "strutils.h"

#include <chrono>
#include <cstring>
#include <memory>
#include <string>

#define SPACE_CHARS " \t\r\n"

#ifndef MAX_RTSP_SIZE
#define MAX_RTSP_SIZE 4096
#endif

using namespace std;
using namespace chrono;

void get_str_until_chars(char *buf, int buf_size, const char *sep, const char **pp)
{
    const char *p = *pp;
    char       *q = buf;

    p += strspn(p, SPACE_CHARS);
    while (!strchr(sep, *p) && (*p != '\0')) {
        if ((q - buf) < buf_size - 1) {
            *q++ = *p;
        }
        p++;
    }
    if (buf_size > 0)
        *q = '\0';
    *pp = p;
}

int rtsp_send_cmd_content(int fd, RTSPContext *ctx, const char *method, const char *uri, const char *headers)
{
    std::shared_ptr<char> msg = rtsp_method_encode(ctx, "OPTIONS", uri, headers);
    if (::send(fd, msg.get(), strlen(msg.get()), 0) <= 0) {
        return -1;
    }

    auto begin_ = high_resolution_clock::now();
    char buffer[MAX_RTSP_SIZE] = {0};
    int  length = 0;

    for (;;) {
        char  ch = '\0';
        char *ptr = buffer;
        for (int i = 0; i < MAX_RTSP_SIZE; i++) {
            int ret = ::recv(fd, &ch, 1, 0);
            if (ret <= 0) {
                printf("ret: %d", ret);
                return ret;
            }

            if (ch == '\n') {
                break;
            } else if (ch != '\r') {
                *ptr++ = ch;
            }
        }
        *ptr = '\0';
        if (buffer[0] == '\0') {
            break;
        }
    }

    auto end_ = high_resolution_clock::now();
    auto costMS = duration_cast<microseconds>(end_ - begin_).count();
    printf("cost: %ld\n", costMS);
    return length;
}

void rtsp_parse_line(RTSPContext *ctx, RTSPMessageHeader *reply, char *buf, RTSPState *rt, const char *method)
{
    const char *p = buf;

    if (string_istart(p, "Session:", &p)) {
        int t;
        get_word_sep(reply->session_id, sizeof(reply->session_id), ";", &p);
        if (string_istart(p, ";timeout=", &p) && (t = strtol(p, NULL, 10)) > 0) {
            reply->timeout = t;
        }
    } else if (string_istart(p, "Content-Length:", &p)) {
        reply->content_length = strtol(p, NULL, 10);
    } else if (string_istart(p, "Transport:", &p)) {
        rtsp_parse_transport(s, reply, p);
    } else if (string_istart(p, "CSeq:", &p)) {
        reply->seq = strtol(p, NULL, 10);
    } else if (string_istart(p, "Range:", &p)) {
        rtsp_parse_range_npt(p, &reply->range_start, &reply->range_end);
    } else if (string_istart(p, "RealChallenge1:", &p)) {
        p += strspn(p, SPACE_CHARS);
        string_cpy(reply->real_challenge, p, sizeof(reply->real_challenge));
    } else if (string_istart(p, "Server:", &p)) {
        p += strspn(p, SPACE_CHARS);
        string_cpy(reply->server, p, sizeof(reply->server));
    } else if (string_istart(p, "Notice:", &p) || string_istart(p, "X-Notice:", &p)) {
        reply->notice = strtol(p, NULL, 10);
    } else if (string_istart(p, "Location:", &p)) {
        p += strspn(p, SPACE_CHARS);
        string_cpy(reply->location, p, sizeof(reply->location));
    } else if (string_istart(p, "WWW-Authenticate:", &p) && rt) {
        p += strspn(p, SPACE_CHARS);
        ff_http_auth_handle_header(&rt->auth_state, "WWW-Authenticate", p);
    } else if (string_istart(p, "Authentication-Info:", &p) && rt) {
        p += strspn(p, SPACE_CHARS);
        ff_http_auth_handle_header(&rt->auth_state, "Authentication-Info", p);
    } else if (string_istart(p, "Content-Base:", &p) && rt) {
        p += strspn(p, SPACE_CHARS);
        if (method && !strcmp(method, "DESCRIBE")) {
            string_cpy(rt->control_uri, p, sizeof(rt->control_uri));
        }
    } else if (string_istart(p, "RTP-Info:", &p) && rt) {
        p += strspn(p, SPACE_CHARS);
        if (method && !strcmp(method, "PLAY")) {
            rtsp_parse_rtp_info(rt, p);
        }
    } else if (string_istart(p, "Public:", &p) && rt) {
        if (strstr(p, "GET_PARAMETER") && method && !strcmp(method, "OPTIONS")) {
            rt->get_parameter_supported = 1;
        }
    } else if (string_istart(p, "x-Accept-Dynamic-Rate:", &p) && rt) {
        p += strspn(p, SPACE_CHARS);
        rt->accept_dynamic_rate = atoi(p);
    } else if (string_istart(p, "Content-Type:", &p)) {
        p += strspn(p, SPACE_CHARS);
        string_cpy(reply->content_type, p, sizeof(reply->content_type));
    } else if (string_istart(p, "com.ses.streamID:", &p)) {
        p += strspn(p, SPACE_CHARS);
        string_cpy(reply->stream_id, p, sizeof(reply->stream_id));
    }
}