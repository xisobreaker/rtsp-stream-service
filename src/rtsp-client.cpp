#include "rtsp-client.h"
#include "rtspenc.h"

#include <arpa/inet.h>
#include <cstdio>
#include <cstring>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

#ifndef INVALID_SOCKET
#define INVALID_SOCKET -1
#endif

#ifndef MAX_BUFFER_SIZE
#define MAX_BUFFER_SIZE 4096
#endif

RTSPClient::RTSPClient()
{
    m_sockfd = INVALID_SOCKET;
    m_rtspInfo = new RTSPUrlInfo;
    memset(m_rtspInfo, 0, sizeof(RTSPUrlInfo));
}

RTSPClient::~RTSPClient()
{
}

void RTSPClient::closesocket()
{
    ::close(m_sockfd);
    m_sockfd = INVALID_SOCKET;
}

bool RTSPClient::connectServer(const char *ip, uint16_t port)
{
    m_sockfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_sockfd == INVALID_SOCKET) {
        return false;
    }

    struct sockaddr_in sock_addr;
    memset(&sock_addr, 0, sizeof(sock_addr));
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_addr.s_addr = inet_addr(ip);
    sock_addr.sin_port = htons(port);

    if (::connect(m_sockfd, (struct sockaddr *)&sock_addr, sizeof(sockaddr_in)) == -1) {
        closesocket();
        return false;
    }
    return true;
}

bool RTSPClient::rtspTransport(std::string url)
{
    char         recvbuf[MAX_BUFFER_SIZE] = {0};
    int16_t      recvlen = 0;
    RTSPContext *ctx = new RTSPContext;
    memset(ctx, 0, sizeof(RTSPContext));

    std::shared_ptr<char> ret = rtsp_method_encode(ctx, "OPTIONS", url.c_str(), NULL);
    while (true) {
        ::send(m_sockfd, ret.get(), strlen(ret.get()), 0);
        for (;;) {
            recvlen = ::recv(m_sockfd, recvbuf, MAX_BUFFER_SIZE, 0);
            if (recvlen <= 0) {
                printf("socket recv error!");
                closesocket();
                return false;
            }
        }
    }
    return true;
}

bool RTSPClient::connect(std::string url)
{
    split_video_url(m_rtspInfo, url.c_str());
    if (!connectServer(m_rtspInfo->hostname, m_rtspInfo->port)) {
        return false;
    }
    if (!rtspTransport(url)) {
        return false;
    }
    return true;
}
