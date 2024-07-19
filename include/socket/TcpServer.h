#pragma once

#include <string>

class TcpServer
{
public:
    TcpServer(uint16_t port);
    ~TcpServer();

private:
    bool reuseaddr(int sockfd);

public:
    bool bind(int flags = 2);
    bool listen();

private:
    uint16_t m_port;
    int      m_sockfd;
};
