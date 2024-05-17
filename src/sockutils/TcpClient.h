#pragma once

#include "rtsputils.h"
#include <string>

class TcpClient
{
public:
    TcpClient();
    ~TcpClient();

public:
    bool connect(const char *ip, uint16_t port);
    int  send(const char *buffer, uint32_t length);
    int  recv(char *buffer, uint32_t length);
    void close();

private:
    int m_sockfd;
};
