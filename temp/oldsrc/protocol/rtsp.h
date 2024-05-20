#pragma once

#include "httpauth.h"
#include "rtspdefs.h"
#include <memory>

/**
 * 接收应答
 * @param fd socker 套接字
 * @param reply
 * @param content_ptr
 * @param method
 */
int rtsp_read_reply(int fd, RTSPMessageHeader *reply, unsigned char **content_ptr, const char *method);