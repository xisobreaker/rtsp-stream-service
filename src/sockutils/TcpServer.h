#pragma once

#include <string>

class TcpServer
{
public:
    TcpServer(uint16_t port);
    ~TcpServer();

private:
public:
    bool listen(int flags = 2);

private:
    uint16_t m_listenPort;
    int      m_sockfd;
};
