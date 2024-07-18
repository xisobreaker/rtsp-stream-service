// =====================================================================================
//  Copyright (C) 2024 by Jiaxing Shao. All rights reserved
//
//  文 件 名:  sdp.h
//  作    者:  Jiaxing Shao, 980853650@qq.com
//  创建时间:  2024-07-09 14:19:49
//  描    述:
// =====================================================================================
#pragma once

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <cstring>
#include <string>

/**********************************************************
 * SDP 源信息。
 **********************************************************/
struct sdp_origin {
    char         *username;
    long long int sess_id;
    long long int sess_version;
    char         *nettype;
    char         *addrtype;
    char         *addr;
};

/**********************************************************
 * SDP 连接信息。
 **********************************************************/
struct sdp_connection {
    char *nettype;
    char *addrtype;
    char *address;
};

/**********************************************************
 * SDP 媒体信息。
 **********************************************************/
struct sdp_bandwidth {
    char *bwtype;
    char *bandwidth;
};

/**********************************************************
 * SDP 媒体信息。
 **********************************************************/
struct sdp_repeat {
    time_t  interval;
    time_t  duration;
    time_t *offsets;
    int     offsets_count;
};

/**********************************************************
 * SDP 时间信息。
 **********************************************************/
struct sdp_time {
    time_t             starttime;
    time_t             stoptime;
    struct sdp_repeat *repeat;
    int                repeat_count;
};

/**********************************************************
 * SDP 媒体信息。
 **********************************************************/
struct sdp_timezone_adjustments {
    time_t adjust;
    time_t offset;
};

/**********************************************************
 * SDP 媒体信息。
 **********************************************************/
struct sdp_mediainfo {
    char *type;
    int   port;
    int   port_n;
    char *proto;
    int  *fmt;
    int   fmt_count;
};

/**********************************************************
 * SDP 媒体信息。
 **********************************************************/
struct sdp_media {
    struct sdp_mediainfo  info;
    char                 *title;
    struct sdp_connection conn;
    struct sdp_bandwidth *bw;
    int                   bw_count;
    char                 *encrypt_key;
    char                **attributes;
    int                   attributes_count;
};

/**********************************************************
 * SDP 负载数据。
 **********************************************************/
struct SDPPayload {
    char                            *_payload;
    unsigned char                    proto_version;
    struct sdp_origin                origin;
    char                            *session_name;
    char                            *session_info;
    char                            *uri;
    char                           **emails;
    int                              emails_count;
    char                           **phones;
    int                              phones_count;
    struct sdp_connection            conn;
    struct sdp_bandwidth            *bw;
    int                              bw_count;
    struct sdp_time                 *times;
    int                              times_count;
    struct sdp_timezone_adjustments *timezone_adj;
    int                              timezone_adj_count;
    char                            *encrypt_key;
    char                           **attributes;
    int                              attributes_count;
    struct sdp_media                *medias;
    int                              medias_count;
};

/**
 * @brief 读取下一节点信息
 *
 * @param p
 * @param key
 * @param value
 * @return char*
 */
char *load_next_entry(char *p, char *key, char **value);

/**
 * @brief sdp 数据解析
 *
 * @param payload
 * @return struct SDPPayload*
 */
struct SDPPayload *sdp_parser(const char *payload);
std::string        sdp_format(const struct SDPPayload *sdp);
void               sdp_destroy(struct SDPPayload *sdp);
