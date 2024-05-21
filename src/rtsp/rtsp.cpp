#include "rtsp.h"
#include "crypto.h"
#include "http_auth.h"
#include "memutils.h"
#include "strutils.h"
#include "timeutils.h"
#include <cmath>
#include <memory>
#include <netdb.h>

#ifndef MAX_RTSP_SIZE
    #define MAX_RTSP_SIZE 4096
#endif

std::string rtsp_method_encode(RTSPContext *ctx, const char *method, const char *uri, const char *headers)
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
        std::string auth = http_auth_create_response(&ctx->auth_state, ctx->auth, uri, method);
        if (!auth.empty()) {
            string_lcat(buf.get(), auth.c_str(), MAX_RTSP_SIZE);
        }
    }
    string_lcat(buf.get(), "\r\n", MAX_RTSP_SIZE);
    return std::string(buf.get());
}

void rtsp_parse_range(int *min_ptr, int *max_ptr, const char **pp)
{
    const char *q = *pp;
    char       *p;
    q += strspn(q, SPACE_CHARS);
    int v = strtol(q, &p, 10);
    if (*p == '-') {
        p++;
        *min_ptr = v;
        v = strtol(p, &p, 10);
        *max_ptr = v;
    } else {
        *min_ptr = v;
        *max_ptr = v;
    }
    *pp = p;
}

int get_sockaddr(const char *buf, struct sockaddr_storage *sock)
{
    struct addrinfo  hints = {0};
    struct addrinfo *ai = NULL;
    int              ret;

    hints.ai_flags = AI_NUMERICHOST;
    if ((ret = getaddrinfo(buf, NULL, &hints, &ai))) {
        printf("getaddrinfo error: %s\n", buf);
        return -1;
    }
    memcpy(sock, ai->ai_addr, sizeof(sockaddr_storage));
    freeaddrinfo(ai);
    return 0;
}

void get_str_until_chars(char *buf, int buf_size, const char *sep, const char **pp)
{
    const char *p = *pp;
    char       *q = buf;

    // 跳过 SPACE_CHARS
    p += strspn(p, SPACE_CHARS);
    // strchr  函数返回指向在 sep 中首次出现 *p 字符的指针，未找到时返回 null
    while (!strchr(sep, *p) && (*p != '\0')) {
        if ((q - buf) < buf_size - 1) {
            *q++ = *p;
        }
        p++;
    }
    if (buf_size > 0) {
        *q = '\0';
    }
    *pp = p;
}

void get_str_skip_slash(char *buf, int buf_size, const char *sep, const char **pp)
{
    if (**pp == '/') {
        (*pp)++;
    }
    get_str_until_chars(buf, buf_size, sep, pp);
}

void rtsp_parse_transport(RTSPContext *ctx, RTSPMessage *reply, const char *p)
{
    char transport_protocol[16] = {0};
    char profile[16] = {0};
    char lower_transport[16] = {0};
    char parameter[16] = {0};
    char buf[256] = {0};

    reply->nb_transports = 0;

    for (;;) {
        p += strspn(p, SPACE_CHARS);
        if (*p == '\0') {
            break;
        }

        RTSPTransportField *transField = &reply->transports[reply->nb_transports];
        get_str_until_chars(transport_protocol, sizeof(transport_protocol), "/", &p);
        if (string_casecmp(transport_protocol, "rtp") == 0) {
            get_str_skip_slash(profile, sizeof(profile), "/;,", &p);
            if (*p == '/') {
                get_str_skip_slash(lower_transport, sizeof(lower_transport), ";,", &p);
            }
            transField->transport = RTSP_TRANSPORT_RTP;
        } else if (!string_casecmp(transport_protocol, "x-pn-tng") || !string_casecmp(transport_protocol, "x-real-rdt")) {
            get_str_skip_slash(lower_transport, sizeof(lower_transport), "/;,", &p);
            transField->transport = RTSP_TRANSPORT_RDT;
        } else if (!string_casecmp(transport_protocol, "raw")) {
            get_str_skip_slash(profile, sizeof(profile), "/;,", &p);
            if (*p == '/') {
                get_str_skip_slash(lower_transport, sizeof(lower_transport), ";,", &p);
            }
            transField->transport = RTSP_TRANSPORT_RAW;
        } else {
            break;
        }

        if (!string_casecmp(lower_transport, "TCP")) {
            transField->lower_transport = RTSP_LOWER_TRANSPORT_TCP;
        } else {
            transField->lower_transport = RTSP_LOWER_TRANSPORT_UDP;
        }

        if (*p == ';') {
            p++;
        }

        while (*p != '\0' && *p != ',') {
            get_str_skip_slash(parameter, sizeof(parameter), "=;,", &p);
            if (!strcmp(parameter, "port")) {
                if (*p == '=') {
                    p++;
                    rtsp_parse_range(&transField->port_min, &transField->port_max, &p);
                }
            } else if (!strcmp(parameter, "client_port")) {
                if (*p == '=') {
                    p++;
                    rtsp_parse_range(&transField->client_port_min, &transField->client_port_max, &p);
                }
            } else if (!strcmp(parameter, "server_port")) {
                if (*p == '=') {
                    p++;
                    rtsp_parse_range(&transField->server_port_min, &transField->server_port_max, &p);
                }
            } else if (!strcmp(parameter, "interleaved")) {
                if (*p == '=') {
                    p++;
                    rtsp_parse_range(&transField->interleaved_min, &transField->interleaved_max, &p);
                }
            } else if (!strcmp(parameter, "multicast")) {
                if (transField->lower_transport == RTSP_LOWER_TRANSPORT_UDP)
                    transField->lower_transport = RTSP_LOWER_TRANSPORT_UDP_MULTICAST;
            } else if (!strcmp(parameter, "ttl")) {
                if (*p == '=') {
                    char *end;
                    p++;
                    transField->ttl = strtol(p, &end, 10);
                    p = end;
                }
            } else if (!strcmp(parameter, "destination")) {
                if (*p == '=') {
                    p++;
                    get_str_skip_slash(buf, sizeof(buf), ";,", &p);
                    get_sockaddr(buf, &transField->destination);
                }
            } else if (!strcmp(parameter, "source")) {
                if (*p == '=') {
                    p++;
                    get_str_skip_slash(buf, sizeof(buf), ";,", &p);
                    string_copy(transField->source, buf, sizeof(transField->source));
                }
            } else if (!strcmp(parameter, "mode")) {
                if (*p == '=') {
                    p++;
                    get_str_skip_slash(buf, sizeof(buf), ";, ", &p);
                    if (!strcmp(buf, "record") || !strcmp(buf, "receive")) {
                        transField->mode_record = 1;
                    }
                }
            }

            while (*p != ';' && *p != '\0' && *p != ',') {
                p++;
            }
            if (*p == ';') {
                p++;
            }
        }
        if (*p == ',') {
            p++;
        }

        reply->nb_transports++;
        if (reply->nb_transports >= RTSP_MAX_TRANSPORTS) {
            break;
        }
    }
}

int parse_timestr(int64_t *timeval, const char *timestr)
{
    if (string_casecmp(timestr, "now")) {
        *timeval = get_current_microseconds();
        return 0;
    }

    //     const char              *p, *q;
    //     int64_t                  t, now64;
    //     time_t                   now;
    //     struct tm                dt = {0}, tmbuf;
    //     int                      today = 0, negative = 0, microseconds = 0,
    //     suffix = 1000000; int                      i; static const char *const
    //     date_fmt[] = {
    //         "%Y - %m - %d",
    //         "%Y%m%d",
    //     };
    //     static const char *const time_fmt[] = {
    //         "%H:%M:%S",
    //         "%H%M%S",
    //     };
    //     static const char *const tz_fmt[] = {
    //         "%H:%M",
    //         "%H%M",
    //         "%H",
    //     };

    //     p = timestr;
    //     q = NULL;
    //     *timeval = INT64_MIN;
    //     if (!duration) {
    //         now64 =
    //         chrono::duration_cast<chrono::microseconds>(high_resolution_clock::now().time_since_epoch()).count();
    //         now = now64 / 1000000;

    //         if (!av_strcasecmp(timestr, "now")) {
    //             *timeval = now64;
    //             return 0;
    //         }

    //         /* parse the year-month-day part */
    //         for (i = 0; i < FF_ARRAY_ELEMS(date_fmt); i++) {
    //             q = av_small_strptime(p, date_fmt[i], &dt);
    //             if (q)
    //                 break;
    //         }

    //         /* if the year-month-day part is missing, then take the
    //          * current year-month-day time */
    //         if (!q) {
    //             today = 1;
    //             q = p;
    //         }
    //         p = q;

    //         if (*p == 'T' || *p == 't')
    //             p++;
    //         else
    //             while (av_isspace(*p))
    //                 p++;

    //         /* parse the hour-minute-second part */
    //         for (i = 0; i < FF_ARRAY_ELEMS(time_fmt); i++) {
    //             q = av_small_strptime(p, time_fmt[i], &dt);
    //             if (q)
    //                 break;
    //         }
    //     } else {
    //         /* parse timestr as a duration */
    //         if (p[0] == '-') {
    //             negative = 1;
    //             ++p;
    //         }
    //         /* parse timestr as HH:MM:SS */
    //         q = av_small_strptime(p, "%J:%M:%S", &dt);
    //         if (!q) {
    //             /* parse timestr as MM:SS */
    //             q = av_small_strptime(p, "%M:%S", &dt);
    //             dt.tm_hour = 0;
    //         }
    //         if (!q) {
    //             char *o;
    //             /* parse timestr as S+ */
    //             errno = 0;
    //             t = strtoll(p, &o, 10);
    //             if (o == p) /* the parsing didn't succeed */
    //                 return AVERROR(EINVAL);
    //             if (errno == ERANGE)
    //                 return AVERROR(ERANGE);
    //             q = o;
    //         } else {
    //             t = dt.tm_hour * 3600 + dt.tm_min * 60 + dt.tm_sec;
    //         }
    //     }

    //     /* Now we have all the fields that we can get */
    //     if (!q)
    //         return AVERROR(EINVAL);

    //     /* parse the .m... part */
    //     if (*q == '.') {
    //         int n;
    //         q++;
    //         for (n = 100000; n >= 1; n /= 10, q++) {
    //             if (!av_isdigit(*q))
    //                 break;
    //             microseconds += n * (*q - '0');
    //         }
    //         while (av_isdigit(*q))
    //             q++;
    //     }

    //     if (duration) {
    //         if (q[0] == 'm' && q[1] == 's') {
    //             suffix = 1000;
    //             microseconds /= 1000;
    //             q += 2;
    //         } else if (q[0] == 'u' && q[1] == 's') {
    //             suffix = 1;
    //             microseconds = 0;
    //             q += 2;
    //         } else if (*q == 's')
    //             q++;
    //     } else {
    //         int is_utc = *q == 'Z' || *q == 'z';
    //         int tzoffset = 0;
    //         q += is_utc;
    //         if (!today && !is_utc && (*q == '+' || *q == '-')) {
    //             struct tm tz = {0};
    //             int       sign = (*q == '+' ? -1 : 1);
    //             q++;
    //             p = q;
    //             for (i = 0; i < FF_ARRAY_ELEMS(tz_fmt); i++) {
    //                 q = av_small_strptime(p, tz_fmt[i], &tz);
    //                 if (q)
    //                     break;
    //             }
    //             if (!q)
    //                 return AVERROR(EINVAL);
    //             tzoffset = sign * (tz.tm_hour * 60 + tz.tm_min) * 60;
    //             is_utc = 1;
    //         }
    //         if (today) { /* fill in today's date */
    //             struct tm dt2 = is_utc ? *gmtime_r(&now, &tmbuf) :
    //             *localtime_r(&now, &tmbuf); dt2.tm_hour = dt.tm_hour;
    //             dt2.tm_min = dt.tm_min;
    //             dt2.tm_sec = dt.tm_sec;
    //             dt = dt2;
    //         }
    //         dt.tm_isdst = is_utc ? 0 : -1;
    //         t = is_utc ? av_timegm(&dt) : mktime(&dt);
    //         t += tzoffset;
    //     }

    //     /* Check that we are at the end of the string */
    //     if (*q)
    //         return AVERROR(EINVAL);

    //     if (INT64_MAX / suffix < t || t < INT64_MIN / suffix)
    //         return AVERROR(ERANGE);
    //     t *= suffix;
    //     if (INT64_MAX - microseconds < t)
    //         return AVERROR(ERANGE);
    //     t += microseconds;
    //     if (t == INT64_MIN && negative)
    //         return AVERROR(ERANGE);
    //     *timeval = negative ? -t : t;
    return 0;
}

void rtsp_parse_range_npt(const char *ptr, int64_t *start, int64_t *end)
{
    char buf[256];
    ptr += strspn(ptr, SPACE_CHARS);
    if (!string_istart(ptr, "npt=", &ptr)) {
        return;
    }

    *start = RTSP_NOPTS_VALUE;
    *end = RTSP_NOPTS_VALUE;

    get_str_until_chars(buf, sizeof(buf), "-", &ptr);
    if (parse_timestr(start, buf)) {
        return;
    } else {
        if (*ptr == '-') {
            ptr++;
            get_str_until_chars(buf, sizeof(buf), "-", &ptr);
            parse_timestr(end, buf);
        }
    }
}

void rtsp_parse_line(RTSPContext *ctx, RTSPMessage *reply, const char *message, const char *method)
{
    const char *ptr = message;
    if (string_istart(ptr, "Session:", &ptr)) {
        get_str_until_chars(reply->session_id, sizeof(reply->session_id), ";", &ptr);
        if (string_istart(ptr, ";timeout", &ptr)) {
            reply->timeout = strtol(ptr, NULL, 10);
        }
    } else if (string_istart(ptr, "Content-Length:", &ptr)) {
        reply->content_length = strtol(ptr, NULL, 10);
    } else if (string_istart(ptr, "Transport:", &ptr)) {
        rtsp_parse_transport(ctx, reply, ptr);
    } else if (string_istart(ptr, "CSeq:", &ptr)) {
        reply->seq = strtol(ptr, NULL, 10);
    } else if (string_istart(ptr, "Range:", &ptr)) {
        rtsp_parse_range_npt(ptr, &reply->range_start, &reply->range_end);
    } else if (string_istart(ptr, "RealChallenge1:", &ptr)) {
        ptr += strspn(ptr, SPACE_CHARS);
        string_copy(reply->real_challenge, ptr, sizeof(reply->real_challenge));
    } else if (string_istart(ptr, "Server:", &ptr)) {
        ptr += strspn(ptr, SPACE_CHARS);
        string_copy(reply->server, ptr, sizeof(reply->server));
    } else if (string_istart(ptr, "Notice:", &ptr) || string_istart(ptr, "X-Notice:", &ptr)) {
        reply->notice = strtol(ptr, NULL, 10);
    } else if (string_istart(ptr, "Location:", &ptr)) {
        ptr += strspn(ptr, SPACE_CHARS);
        string_copy(reply->location, ptr, sizeof(reply->location));
    } else if (string_istart(ptr, "WWW-Authenticate:", &ptr)) {
        ptr += strspn(ptr, SPACE_CHARS);
        http_auth_handle_header(&ctx->auth_state, "WWW-Authenticate", ptr);
    } else if (string_istart(ptr, "Authentication-Info:", &ptr)) {
        ptr += strspn(ptr, SPACE_CHARS);
        http_auth_handle_header(&ctx->auth_state, "Authentication-Info", ptr);
    } else if (string_istart(ptr, "Content-Base:", &ptr)) {
        ptr += strspn(ptr, SPACE_CHARS);
        if (method && !strcmp(method, "DESCRIBE")) {
            string_copy(ctx->base_uri, ptr, sizeof(ctx->base_uri));
        }
    } else if (string_istart(ptr, "RTP-Info:", &ptr)) {
        ptr += strspn(ptr, SPACE_CHARS);
        if (method && !strcmp(method, "PLAY")) {
            // rtsp_parse_rtp_info(rt, p);
        }
    } else if (string_istart(ptr, "Public:", &ptr)) {
        if (strstr(ptr, "GET_PARAMETER") && method && !strcmp(method, "OPTIONS")) {
            ctx->get_parameter_supported = 1;
        }
    } else if (string_istart(ptr, "x-Accept-Dynamic-Rate:", &ptr)) {
        ptr += strspn(ptr, SPACE_CHARS);
        ctx->accept_dynamic_rate = atoi(ptr);
    } else if (string_istart(ptr, "Content-Type:", &ptr)) {
        ptr += strspn(ptr, SPACE_CHARS);
        string_copy(reply->content_type, ptr, sizeof(reply->content_type));
    } else if (string_istart(ptr, "com.ses.streamID:", &ptr)) {
        ptr += strspn(ptr, SPACE_CHARS);
        string_copy(reply->stream_id, ptr, sizeof(reply->stream_id));
    }
}

void http_auth_handle_header(HTTPAuthState *state, const char *key, const char *value)
{
    if (!string_casecmp(key, "WWW-Authenticate") || !string_casecmp(key, "Proxy-Authenticate")) {
        const char *p;
        if (string_istart(value, "Basic ", &p) && state->auth_type <= HTTP_AUTH_BASIC) {
            state->auth_type = HTTP_AUTH_BASIC;
            auto vecStrs = string_split(p, ",", true);
            for (const auto &str : vecStrs) {
                const char *ptr = nullptr;
                if (string_istart(str.c_str(), "realm=", &ptr)) {
                    std::string s = string_trim(ptr, '\"');
                    strncpy(state->realm, s.c_str(), s.length());
                }
            }
        } else if (string_istart(value, "Digest ", &p) && state->auth_type <= HTTP_AUTH_DIGEST) {
            state->auth_type = HTTP_AUTH_DIGEST;
            auto vecStrs = string_split(p, ",", true);
            for (const auto &str : vecStrs) {
                const char *ptr = nullptr;
                if (string_istart(str.c_str(), "realm=", &ptr)) {
                    std::string s = string_trim(ptr, '\"');
                    strncpy(state->realm, s.c_str(), s.length());
                } else if (string_istart(str.c_str(), "nonce=", &ptr)) {
                    std::string s = string_trim(ptr, '\"');
                    strncpy(state->digest_params.nonce, s.c_str(), s.length());
                }
            }
        }
    } else if (string_casecmp(key, "Authentication-Info")) {
        auto vecStrs = string_split(value, ",", true);
        for (const auto &str : vecStrs) {
            const char *ptr = nullptr;
            if (string_istart(str.c_str(), "nextnonce=", &ptr)) {
                std::string s = string_trim(ptr, '\"');
                strncpy(state->digest_params.nonce, s.c_str(), s.length());
            }
        }
    }
}

int rtsp_read_reply(int fd, RTSPMessage *reply, unsigned char **content_ptr, const char *method)
{
    char line_buf[MAX_RTSP_SIZE] = {0};
    int  line_len = 0;

    for (;;) {
        char  ch = '\0';
        char *ptr = line_buf;
        for (int i = 0; i < MAX_RTSP_SIZE; i++) {
            int ret = ::recv(fd, &ch, 1, 0);
            if (ret <= 0) {
                return ret;
            }

            if (ch == '\n') {
                break;
            } else if (ch != '\r') {
                *ptr++ = ch;
            }
        }
        *ptr = '\0';

        if (line_buf[0] == '\0') {
            break;
        }

        if (line_len == 0) {
        } else {
        }
        line_len++;
    }
    return 0;
}