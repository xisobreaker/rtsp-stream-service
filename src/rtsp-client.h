#pragma once

#include "rtsputils.h"
#include <string>

class RTSPClient
{
public:
    RTSPClient();
    ~RTSPClient();

private:
    void closesocket();
    bool connectServer(const char *ip, uint16_t port);
    bool rtspTransport(std::string url);

public:
    bool connect(std::string url);

private:
    int          m_sockfd;
    RTSPUrlInfo *m_rtspInfo;
};
