#include "rtsp.h"
#include "crypto.h"
#include "http_auth.h"
#include "memutils.h"
#include "rtsp_def.h"
#include "strutils.h"
#include "timeutils.h"
#include <cmath>
#include <glog/logging.h>
#include <memory>
#include <netdb.h>
#include <string>

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

void rtsp_parse_range(int *min_ptr, int *max_ptr, const std::string &msg)
{
    auto vec = string_split(msg, "-");
    if (vec.size() == 2) {
        *min_ptr = std::stoi(vec[0]);
        *max_ptr = std::stoi(vec[1]);
    } else {
        *min_ptr = std::stoi(vec[0]);
        *max_ptr = std::stoi(vec[0]);
    }
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

void rtsp_parse_transport(RTSPContext *ctx, RTSPMessage *reply, const std::string &msg)
{
    reply->nb_transports = 0;
    std::string lower_transport = "UDP";

    auto vecTransports = string_split(msg, "\r\n");
    for (int i = 0; i < vecTransports.size(); i++) {
        if (reply->nb_transports >= RTSP_MAX_TRANSPORTS)
            break;

        RTSPTransportField *transport = &reply->transports[reply->nb_transports];
        std::string         transport_protocol = string_cut_until_char(vecTransports[i], "/");
        if (transport_protocol == "RTP") {
            std::string profile = string_cut_until_char(vecTransports[i], "/;,", true);
            if (vecTransports[i].at(0) == '/') {
                vecTransports[i] = vecTransports[i].substr(1, vecTransports[i].length());
                lower_transport = string_cut_until_char(vecTransports[i], ";,");
            } else if (vecTransports[i].at(0) == ';' || vecTransports[i].at(0) == ',') {
                vecTransports[i] = vecTransports[i].substr(1, vecTransports[i].length());
            }
            transport->transport = RTSP_TRANSPORT_RTP;
        } else {
            LOG(WARNING) << "unknown transport protocol: " << transport_protocol;
        }

        if (lower_transport == "TCP") {
            transport->lower_transport = RTSP_LOWER_TRANSPORT_TCP;
        } else {
            transport->lower_transport = RTSP_LOWER_TRANSPORT_UDP;
        }

        auto vecStrs = string_split(vecTransports[i], ";");
        for (auto &field : vecStrs) {
            std::string str = string_cut_until_char(field, "=");
            if (str == "multicast") {
                if (transport->lower_transport == RTSP_LOWER_TRANSPORT_UDP) {
                    transport->lower_transport = RTSP_LOWER_TRANSPORT_UDP_MULTICAST;
                }
            } else if (str == "port") {
                rtsp_parse_range(&transport->port_min, &transport->port_max, field);
            } else if (str == "client_port") {
                rtsp_parse_range(&transport->client_port_min, &transport->client_port_max, field);
            } else if (str == "server_port") {
                rtsp_parse_range(&transport->server_port_min, &transport->server_port_max, field);
            } else if (str == "interleaved") {
                rtsp_parse_range(&transport->interleaved_min, &transport->interleaved_max, field);
            } else if (str == "ttl") {
                transport->ttl = std::stoi(field);
            } else if (str == "destination") {
                get_sockaddr(field.c_str(), &transport->destination);
            } else if (str == "source") {
                snprintf(transport->source, sizeof(transport->source), "%s", field.c_str());
            } else if (str == "mode") {
                if (field == "record" || field == "receive") {
                    transport->mode_record = 1;
                }
            }
        }
        reply->nb_transports++;
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

void rtsp_parse_line(RTSPContext *ctx, RTSPMessage *reply, const char *msg, const char *method)
{
    std::string message = msg;
    if (string_start_and_cut(message, "Authentication-Info:")) {
        http_auth_handle_header(&ctx->auth_state, "Authentication-Info", message);
    } else if (string_start_and_cut(message, "Cache-Control:")) {
        LOG(WARNING) << "Cache-Control: " << message;
    } else if (string_start_and_cut(message, "Content-Base:")) {
        snprintf(ctx->base_uri, sizeof(ctx->base_uri), "%s", message.c_str());
    } else if (string_start_and_cut(message, "Content-Length:")) {
        reply->content_length = std::stoi(message);
    } else if (string_start_and_cut(message, "Content-Type:")) {
        snprintf(reply->content_type, sizeof(reply->content_type), "%s", message.c_str());
    } else if (string_start_and_cut(message, "CSeq:")) {
        reply->seq = std::stoi(message);
    } else if (string_start_and_cut(message, "Location:")) {
        snprintf(reply->location, sizeof(reply->location), "%s", message.c_str());
    } else if (string_start_and_cut(message, "Public:")) {
        if (message.find("GET_PARAMETER") != std::string::npos && !strcmp(method, "OPTIONS")) {
            ctx->get_parameter_supported = 1;
        }
    } else if (string_start_and_cut(message, "Range:")) {
        // rtsp_parse_range_npt(ptr, &reply->range_start, &reply->range_end);
    } else if (string_start_and_cut(message, "RTP-Info:")) {
        if (method && !strcmp(method, "PLAY")) {
            // rtsp_parse_rtp_info(rt, p);
        }
    } else if (string_start_and_cut(message, "Session:")) {
        std::string session_id = string_cut_until_char(message, ";");
        snprintf(reply->session_id, sizeof(reply->session_id), "%s", session_id.c_str());
        if (string_start_and_cut(message, "timeout=")) {
            reply->timeout = std::stoi(message);
        }
    } else if (string_start_and_cut(message, "Server:")) {
        snprintf(reply->server, sizeof(reply->server), "%s", message.c_str());
    } else if (string_start_and_cut(message, "Transport:")) {
        rtsp_parse_transport(ctx, reply, message);
    } else if (string_start_and_cut(message, "Notice:") || string_start_and_cut(message, "X-Notice:")) {
        reply->notice = std::stoi(message);
    } else if (string_start_and_cut(message, "WWW-Authenticate:")) {
        http_auth_handle_header(&ctx->auth_state, "WWW-Authenticate", message);
    } else if (string_start_and_cut(message, "x-Accept-Dynamic-Rate:")) {
        ctx->accept_dynamic_rate = std::stoi(message);
    } else {
        LOG(WARNING) << "unknown header: " << message;
    }
}

void http_auth_handle_header(HTTPAuthState *state, const std::string &key, const std::string &value)
{
    if (key == "WWW-Authenticate" || key == "Proxy-Authenticate") {
        std::string message = value;
        if (string_start_and_cut(message, "Basic ") && state->auth_type <= HTTP_AUTH_BASIC) {
            state->auth_type = HTTP_AUTH_BASIC;
            auto vecStrs = string_split(message, ",", true);
            for (const auto &str : vecStrs) {
                const char *ptr = nullptr;
                if (string_istart(str.c_str(), "realm=", &ptr)) {
                    std::string s = string_trim(ptr, '\"');
                    strncpy(state->realm, s.c_str(), s.length());
                }
            }
        } else if (string_start_and_cut(message, "Digest ") && state->auth_type <= HTTP_AUTH_DIGEST) {
            state->auth_type = HTTP_AUTH_DIGEST;
            auto vecStrs = string_split(message, ",", true);
            for (auto &str : vecStrs) {
                if (string_start_and_cut(str, "realm=")) {
                    std::string s = string_trim(str, '\"');
                    strncpy(state->realm, s.c_str(), s.length());
                } else if (string_start_and_cut(str, "nonce=")) {
                    std::string s = string_trim(str, '\"');
                    strncpy(state->digest_params.nonce, s.c_str(), s.length());
                }
            }
        }
    } else if (key == "Authentication-Info") {
        auto vecStrs = string_split(value, ",", true);
        for (auto &str : vecStrs) {
            if (string_start_and_cut(str, "nextnonce=")) {
                std::string s = string_trim(str, '\"');
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