#include "TcpClient.h"

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

TcpClient::TcpClient()
{
    m_sockfd = INVALID_SOCKET;
}

TcpClient::~TcpClient()
{
}

void TcpClient::close()
{
    ::close(m_sockfd);
    m_sockfd = INVALID_SOCKET;
}

bool TcpClient::connect(const char *ip, uint16_t port)
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

    if (::connect(m_sockfd, (struct sockaddr *)&sock_addr, sizeof(sockaddr_in)) == SOCKET_ERROR) {
        this->close();
        return false;
    }
    return true;
}

int TcpClient::send(const char *buffer, unsigned int length)
{
    int sendLen = 0;
    do {
        int ret = ::send(m_sockfd, buffer + sendLen, length - sendLen, 0);
        if (ret <= 0) {
            return -1;
        }

        sendLen += ret;
    } while (sendLen == length);
    return sendLen;
}

int TcpClient::recv(char *buffer, uint32_t length)
{
    return ::recv(m_sockfd, buffer, length, 0);
}