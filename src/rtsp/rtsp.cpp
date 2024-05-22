#include "rtsp.h"
#include "crypto.h"
#include "parse.h"
#include "strutils.h"
#include <glog/logging.h>
#include <memory>

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
