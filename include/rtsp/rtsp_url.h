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