#pragma once

#include <string>

class RTSPServer
{
public:
    RTSPServer(uint16_t port);
    ~RTSPServer();

public:
    bool listen(int flags = 2);

private:
    uint16_t m_listenPort;
    int      m_sockfd;
};
