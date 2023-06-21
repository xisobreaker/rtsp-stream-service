#pragma once

#include <string>

class RTSPClient
{
public:
    RTSPClient();
    ~RTSPClient();

private:
    bool connectServer();
    void closesocket();

public:
    bool connect(std::string url);

private:
    char     m_ipaddr[16];
    uint16_t m_port;
    int      m_sockfd;
};
