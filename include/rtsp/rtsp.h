#pragma once

#include "structs.h"

/**
 * 命令数据编码
 * @param ctx
 * @param method
 * @param uri
 * @param headers
 */
std::string rtsp_method_encode(RTSPContext *ctx, const char *method, const char *uri, const char *headers);

/**
 * 解析行数据
 * @param ctx
 * @param reply
 * @param message
 * @param rt
 * @param method
 */
void rtsp_parse_line(RTSPContext *ctx, RTSPMessage *reply, const char *msg, const char *method);
