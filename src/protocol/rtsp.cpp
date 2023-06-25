#include "rtsp.h"

#include <cstring>
#include <memory>
#include <string>

#ifndef MAX_RTSP_SIZE
#define MAX_RTSP_SIZE 4096
#endif

int rtsp_send_cmd_content(int fd, RTSPContext *ctx, const char *method, const char *uri, const char *headers)
{
    std::shared_ptr<char> msg = rtsp_method_encode(ctx, "OPTIONS", uri, headers);
    if (::send(fd, msg.get(), strlen(msg.get()), 0) <= 0) {
        return -1;
    }

    char buffer[MAX_RTSP_SIZE] = {0};
    int  ret = ::recv(fd, buffer, MAX_RTSP_SIZE, 0);
    if (ret <= 0) {
        return ret;
    }
    return 0;
}
