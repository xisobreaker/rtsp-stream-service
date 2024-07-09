// =====================================================================================
//  Copyright (C) 2024 by Jiaxing Shao. All rights reserved
//
//  文 件 名:  rtsp_url.h
//  作    者:  Jiaxing Shao, 980853650@qq.com
//  创建时间:  2024-07-09 14:19:36
//  描    述:
// =====================================================================================
#pragma once

#include <cstdint>

/**********************************************************
 * RTSP url 地址信息
 **********************************************************/
typedef struct {
    char     proto[16];          // 协议类型
    char     authorization[128]; // 授权认证
    char     hostname[16];       // 地址
    uint16_t port;               // 端口
    char     path[512];          // 路径
} RTSPUrlInfo;

/**
 * 解析视频 url 地址
 * @param info 解析后的信息
 * @param url  视频url地址
 */
void split_video_url(RTSPUrlInfo *info, const char *url);