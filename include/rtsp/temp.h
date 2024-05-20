#pragma once

#include "rtsp.h"

/**
 * 接收应答
 * @param fd socker 套接字
 * @param reply
 * @param content_ptr
 * @param method
 */
int rtsp_read_reply(int fd, RTSPMessage *reply, unsigned char **content_ptr, const char *method);