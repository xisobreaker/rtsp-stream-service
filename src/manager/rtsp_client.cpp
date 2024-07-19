#include "rtsp_client.h"
#include "rtsp.h"
#include "rtsp_url.h"
#include "sdp.h"
#include "structs.h"
#include <glog/logging.h>

RtspClient::RtspClient(std::string rtspurl) : m_rtspurl(rtspurl)
{
    m_info = new RTSPUrlInfo;
    memset(m_info, 0, sizeof(RTSPUrlInfo));

    m_ctx = new RTSPContext;
    memset(m_ctx, 0, sizeof(RTSPContext));
}

RtspClient::~RtspClient()
{
}

int RtspClient::read_line(char *buffer, int maxlen)
{
    int length = 0;
    do {
        int ret = m_client.recv(buffer + length, 1);
        if (ret <= 0)
            return -1;

        if (buffer[length] == '\r') {
        } else if (buffer[length] == '\n') {
            buffer[length] = '\0';
            return length;
        } else {
            length += ret;
        }
    } while (length < maxlen);
    return -1;
}

bool RtspClient::read_rtsp_message(std::vector<std::string> &lines)
{
    char buffer[4096];
    int  buflen = 0;
    do {
        buflen = read_line(buffer, 4096);
        if (buflen < 0)
            return false;
        if (buflen > 1) {
            lines.push_back(std::string(buffer));
        }
    } while (buflen > 0);
    return true;
}

bool RtspClient::rtsp_interactive(std::vector<std::string> &lines, const char *method, const char *header)
{
    std::string rtspcmd = rtsp_method_encode(m_ctx, method, m_info->path, header);
    int         ret     = m_client.send(rtspcmd.c_str(), rtspcmd.length());
    if (ret <= 0) {
        LOG(INFO) << "send error: " << ret;
        return false;
    } else {
        LOG(INFO) << "send: \n" << rtspcmd;
    }

    if (!read_rtsp_message(lines)) {
        return false;
    }

    for (int i = 0; i < lines.size(); i++) {
        LOG(INFO) << "line[" << i << "]: " << lines[i];
    }
    return true;
}

bool RtspClient::parse_reply(RTSPMessage *reply, const std::vector<std::string> &lines, const char *method)
{
    for (int i = 0; i < lines.size(); i++) {
        if (i == 0) {
        } else {
            rtsp_parse_line(m_ctx, reply, lines[i].c_str(), method);
        }
    }
    return true;
}

bool RtspClient::rtsp_send_options()
{
    RTSPMessage             *reply = new RTSPMessage;
    std::vector<std::string> lines;

    if (!rtsp_interactive(lines, "OPTIONS", nullptr))
        return false;
    if (!parse_reply(reply, lines, "OPTIONS"))
        return false;
    return true;
}

bool RtspClient::rtsp_send_describe()
{
    RTSPMessage             *reply = new RTSPMessage;
    std::vector<std::string> lines;

    if (!rtsp_interactive(lines, "DESCRIBE", "Accept: application/sdp\r\n"))
        return false;
    if (!parse_reply(reply, lines, "DESCRIBE"))
        return false;

    char buf[4096] = {0};
    if (m_client.recv(buf, reply->content_length) <= 0) {
        LOG(WARNING) << "recv content error: " << buf;
        return false;
    }
    LOG(INFO) << "content: \n" << buf;
    struct SDPPayload *sdp = sdp_parser(buf);
    sdp_print(sdp);
    return true;
}

bool RtspClient::rtsp_send_setup()
{
    RTSPMessage             *reply = new RTSPMessage;
    std::vector<std::string> lines;

    if (!rtsp_interactive(lines, "SETUP", nullptr))
        return false;
    if (!parse_reply(reply, lines, "SETUP"))
        return false;
    return true;
}

bool RtspClient::rtsp_send_play()
{
    RTSPMessage             *reply = new RTSPMessage;
    std::vector<std::string> lines;

    if (!rtsp_interactive(lines, "PLAY", nullptr))
        return false;
    if (!parse_reply(reply, lines, "PLAY"))
        return false;
    return true;
}

bool RtspClient::rtsp_connect()
{
    char buffer[2048] = {0};
    if (!rtsp_send_options()) {
        return false;
    }
    if (!rtsp_send_describe()) {
        return false;
    }
    if (!rtsp_send_setup()) {
        return false;
    }
    if (!rtsp_send_play()) {
        return false;
    }
    return false;
}

bool RtspClient::connect()
{
    memset(m_info, 0, sizeof(RTSPUrlInfo));
    split_video_url(m_info, m_rtspurl.c_str());
    if (!m_client.connect(m_info->hostname, m_info->port)) {
        LOG(ERROR) << "connect server failed: " << m_rtspurl;
        return false;
    }

    memset(m_ctx, 0, sizeof(RTSPContext));
    if (!rtsp_connect()) {
        LOG(ERROR) << "rtsp connect failed: " << m_rtspurl;
        return false;
    }
    return true;
}
