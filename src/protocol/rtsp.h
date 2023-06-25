#pragma once

#include "rtspenc.h"

int rtsp_send_cmd_content(RTSPContext *ctx, const char *method, const char *uri, const char *headers);