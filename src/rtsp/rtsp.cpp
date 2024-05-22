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

/**
 * 网络地址转换
 */
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

int parse_timestr(int64_t *timeval, const std::string &timestr)
{
    int64_t nowTime = get_current_microseconds();
    if (timestr == "now") {
        *timeval = nowTime;
        return 0;
    }

    double value = std::stod(timestr);
    *timeval = nowTime + (value * 1000000);
    return 0;
}

void rtsp_parse_range_npt(const std::string &msg, int64_t *start, int64_t *end)
{
    std::string message = msg;
    if (!string_start_and_cut(message, "npt="))
        return;

    *start = RTSP_NOPTS_VALUE;
    *end = RTSP_NOPTS_VALUE;

    auto vec = string_split(message, "-");
    if (vec.size() >= 1) {
        parse_timestr(start, vec[0]);
    }

    if (vec.size() >= 2) {
        parse_timestr(end, vec[1]);
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
        rtsp_parse_range_npt(message, &reply->range_start, &reply->range_end);
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