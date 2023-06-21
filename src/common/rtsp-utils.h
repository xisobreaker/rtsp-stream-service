#pragma once

#include "rtsp-defines.h"
#include <memory>

#ifndef MAX_FORMAT_SIZE
#define MAX_FORMAT_SIZE 2048
#endif

// 解析视频 url 地址
void split_video_url(RTSPUrlInfo *info, const char *url);

// 生成 digest 加密串
std::shared_ptr<char> make_digest_auth(
    HTTPAuthState *state, const char *username, const char *password, const char *uri, const char *method);
