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

bool RtspClient::rtsp_parseline()
{
    std::string command = "OPTIONS";
    while (true) {
        std::string rtspcmd = rtsp_method_encode(m_ctx, command.c_str(), m_info->path, nullptr);
        if (m_client.send(rtspcmd.c_str(), rtspcmd.length()) <= 0) {
            return false;
        }

        char buffer[4096] = {0};
        int  length = 0;
        while (true) {
            int ret = m_client.recv(buffer + length, 1);
            length += ret;
        }
    }
}

bool RtspClient::rtsp_connect()
{
    char buffer[2048] = {0};
    while (true) {
        rtsp_parseline();
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
