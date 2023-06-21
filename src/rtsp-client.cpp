#include "rtsp-client.h"

#include <arpa/inet.h>
#include <cstring>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

#ifndef INVALID_SOCKET
#define INVALID_SOCKET -1
#endif

RTSPClient::RTSPClient()
{
    m_sockfd = INVALID_SOCKET;
}

RTSPClient::~RTSPClient()
{
}

bool RTSPClient::connectServer()
{
    m_sockfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_sockfd == INVALID_SOCKET) {
        return false;
    }

    struct sockaddr_in sock_addr;
    memset(&sock_addr, 0, sizeof(sock_addr));
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_addr.s_addr = inet_addr(m_ipaddr);
    sock_addr.sin_port = htons(m_port);

    if (::connect(m_sockfd, (struct sockaddr *)&sock_addr, sizeof(sockaddr_in)) == -1) {
        closesocket();
        return false;
    }
    return true;
}

void RTSPClient::closesocket()
{
    ::close(m_sockfd);
    m_sockfd = INVALID_SOCKET;
}

bool RTSPClient::connect(std::string url)
{
    return true;
}
