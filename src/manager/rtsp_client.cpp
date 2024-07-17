#include "rtsp_client.h"
#include "rtsp.h"
#include "rtsp_url.h"
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
            return ret;

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

bool RtspClient::read_rtsp_message(RTSPMessage *reply, const char *method)
{
    char buffer[4096];
    int  buflen = 0;
    do {
        int ret = read_line(buffer, 4096);
        if (ret <= 0)
            return false;
        if (ret > 1) {
            LOG(INFO) << "get line: " << buffer;
            rtsp_parse_line(m_ctx, reply, buffer, method);
        }
    } while (buflen > 0);
    return true;
}

bool RtspClient::rtsp_interactive(const char *method)
{
    RTSPMessage *reply = new RTSPMessage;
    do {
        std::string rtspcmd = rtsp_method_encode(m_ctx, method, m_info->path, nullptr);
        int         ret     = m_client.send(rtspcmd.c_str(), rtspcmd.length());
        if (ret <= 0) {
            LOG(INFO) << "send error: " << ret;
            return false;
        }
        if (!read_rtsp_message(reply, method)) {
            return false;
        }
    } while (reply->status_code == RTSP_STATUS_UNAUTHORIZED);
    return true;
}

bool RtspClient::rtsp_connect()
{
    char buffer[2048] = {0};

    if (!rtsp_interactive("OPTIONS")) {
        return false;
    }
    if (!rtsp_interactive("DESCRIBE")) {
        return false;
    }
    if (!rtsp_interactive("SETUP")) {
        return false;
    }
    if (!rtsp_interactive("PLAY")) {
        return false;
    }

    return false;
}

bool RtspClient::connect()
{
    memset(m_info, 0, sizeof(RTSPUrlInfo));
    split_video_url(m_info, m_rtspurl.c_str());
    LOG(INFO) << "connect server: " << m_info->hostname << ", " << m_info->port;
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
