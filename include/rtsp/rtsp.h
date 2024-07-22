// =====================================================================================
//  Copyright (C) 2024 by Jiaxing Shao. All rights reserved
//
//  文 件 名:  rtsp.h
//  作    者:  Jiaxing Shao, 980853650@qq.com
//  创建时间:  2024-07-09 14:19:42
//  描    述:
// =====================================================================================
#pragma once

#include "sdp.h"
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

void rtsp_sdp_to_sdpinfo(const struct SDPPayload *sdp, RTSPStreamInfo *&info);