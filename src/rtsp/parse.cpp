#include "parse.h"
#include "strutils.h"
#include "timeutils.h"
#include <glog/logging.h>
#include <netdb.h>

void rtsp_parse_range(int *min_ptr, int *max_ptr, const std::string &msg)
{
    auto vec = str_split(msg, "-");
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
    struct addrinfo *ai    = NULL;
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
    reply->nb_transports        = 0;
    std::string lower_transport = "UDP";

    auto vecTransports = str_split(msg, "\r\n");
    for (int i = 0; i < vecTransports.size(); i++) {
        if (reply->nb_transports >= RTSP_MAX_TRANSPORTS)
            break;

        RTSPTransportField *transport          = &reply->transports[reply->nb_transports];
        std::string         transport_protocol = str_cut_until_char(vecTransports[i], "/");
        if (transport_protocol == "RTP") {
            std::string profile = str_cut_until_char(vecTransports[i], "/;,", true);
            if (vecTransports[i].at(0) == '/') {
                vecTransports[i] = vecTransports[i].substr(1, vecTransports[i].length());
                lower_transport  = str_cut_until_char(vecTransports[i], ";,");
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

        auto vecStrs = str_split(vecTransports[i], ";");
        for (auto &field : vecStrs) {
            std::string str = str_cut_until_char(field, "=");
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
    *timeval     = nowTime + (value * 1000000);
    return 0;
}

void rtsp_parse_range_npt(const std::string &msg, int64_t *start, int64_t *end)
{
    std::string message = msg;
    if (!str_start_and_cut(message, "npt="))
        return;

    *start = RTSP_NOPTS_VALUE;
    *end   = RTSP_NOPTS_VALUE;

    auto vec = str_split(message, "-");
    if (vec.size() >= 1) {
        parse_timestr(start, vec[0]);
    }

    if (vec.size() >= 2) {
        parse_timestr(end, vec[1]);
    }
}

void http_auth_handle_header(HTTPAuthState *state, const std::string &key, const std::string &value)
{
    if (key == "WWW-Authenticate" || key == "Proxy-Authenticate") {
        std::string message = value;
        if (str_start_and_cut(message, "Basic ") && state->auth_type <= HTTP_AUTH_BASIC) {
            state->auth_type = HTTP_AUTH_BASIC;
            auto vecStrs     = str_split(message, ",", true);
            for (const auto &str : vecStrs) {
                const char *ptr = nullptr;
                if (str_istart(str.c_str(), "realm=", &ptr)) {
                    std::string s = str_trim(ptr, '\"');
                    strncpy(state->realm, s.c_str(), s.length());
                }
            }
        } else if (str_start_and_cut(message, "Digest ") && state->auth_type <= HTTP_AUTH_DIGEST) {
            state->auth_type = HTTP_AUTH_DIGEST;
            auto vecStrs     = str_split(message, ",", true);
            for (auto &str : vecStrs) {
                if (str_start_and_cut(str, "realm=")) {
                    std::string s = str_trim(str, '\"');
                    strncpy(state->realm, s.c_str(), s.length());
                } else if (str_start_and_cut(str, "nonce=")) {
                    std::string s = str_trim(str, '\"');
                    strncpy(state->digest_params.nonce, s.c_str(), s.length());
                }
            }
        }
    } else if (key == "Authentication-Info") {
        auto vecStrs = str_split(value, ",", true);
        for (auto &str : vecStrs) {
            if (str_start_and_cut(str, "nextnonce=")) {
                std::string s = str_trim(str, '\"');
                strncpy(state->digest_params.nonce, s.c_str(), s.length());
            }
        }
    }
}
