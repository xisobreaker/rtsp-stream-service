#pragma once

#include "rtspdefs.h"
#include <memory>

// 解析视频 url 地址
void split_video_url(RTSPUrlInfo *info, const char *url);
