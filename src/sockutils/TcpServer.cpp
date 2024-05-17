#include "TcpServer.h"

#include <arpa/inet.h>
#include <cstring>
#include <sys/socket.h>
#include <unistd.h>

#ifndef INVALID_SOCKET
    #define INVALID_SOCKET -1
#endif

#ifndef SOCKET_ERROR
    #define SOCKET_ERROR -1
#endif

TcpServer::TcpServer(uint16_t port) : m_listenPort(port)
{
    m_sockfd = INVALID_SOCKET;
}

TcpServer::~TcpServer()
{
}

bool TcpServer::reuseaddr()
{
    int opt = 1;
    if (::setsockopt(m_sockfd, SOL_SOCKET, SO_REUSEADDR, (void *)&opt, sizeof(opt)) == -1) {
        return false;
    }
    return true;
}

bool TcpServer::listen(int flags)
{
    m_sockfd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_sockfd == INVALID_SOCKET) {
        return false;
    }

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(m_listenPort);

    if (flags & SO_REUSEADDR) {
        reuseaddr();
    }

    if (::bind(m_sockfd, (sockaddr *)&serv_addr, sizeof(serv_addr)) == SOCKET_ERROR) {
        ::close(m_sockfd);
        return false;
    }

    if (::listen(m_sockfd, 5) == SOCKET_ERROR) {
        ::close(m_sockfd);
        return false;
    }
    return true;
}
