// =====================================================================================
//  Copyright (C) 2024 by Jiaxing Shao. All rights reserved
//
//  文 件 名:  structs.h
//  作    者:  Jiaxing Shao, 980853650@qq.com
//  创建时间:  2024-07-09 14:19:56
//  描    述:
// =====================================================================================
#pragma once

#include "http_auth.h"
#include "rtsp_def.h"
#include <arpa/inet.h>

#define RTSP_MAX_TRANSPORTS 8
#define RTSP_MAX_REPLY_LINE 32

/**********************************************************
 * RTSP Context。
 **********************************************************/
struct RTSPContext {
    int           seq;        // message sequence
    char         *user_agent; // 用户代理
    char          session_id[512];
    char          auth[128]; // auth check, username:password
    HTTPAuthState auth_state;
    char          base_uri[4096];
    int           get_parameter_supported;
    int           accept_dynamic_rate;
};

/**********************************************************
 * RTSP Transport。
 **********************************************************/
struct RTSPTransportField {
    /** interleave ids, 如果是TCP方式传输数据，每个 TCP/RTSP 数据包头以魔数 '$'，
     * stream 长度和 stream ID。 如果 stream ID 是在 interleaved_min-max 的
     * 范围内, 则此数据包属于该流。 */
    int              interleaved_min, interleaved_max;
    int              port_min, port_max;               // UDP 多播端口范围
    int              client_port_min, client_port_max; // UDP 客户端端口
    int              server_port_min, server_port_max; // UDP 单播端口范围
    int              ttl;
    int              mode_record;
    sockaddr_storage destination;
    char             source[INET6_ADDRSTRLEN + 1];

    RTSPTransport      transport;       // 数据包传输协议
    RTSPLowerTransport lower_transport; // 网络层传输协议
};

/**********************************************************
 * RTSP Message。
 **********************************************************/
struct RTSPMessage {
    enum RTSPStatusCode status_code;            // rtsp 状态码
    int                 seq;                    // RTSP 包序号
    int64_t             range_start, range_end; // 服务将传输的流的时间范围
    int                 content_length;         // 内容长度
    char                session_id[512];
    int                 timeout;
    char                location[4096];
    char                server[64];
    int                 notice;
    char                content_type[64];
    int                 nb_transports; // transports 数量
    RTSPTransportField  transports[RTSP_MAX_TRANSPORTS];
};

/**********************************************************
 * RTSP Reply。
 **********************************************************/
struct RTSPReply {
    int   line_count;
    char *line_data[RTSP_MAX_REPLY_LINE];
    char *content;
    int   content_len;
};

typedef struct {
    int           stream_type; // 0,audio; 1, video; 2, application; 3, text
    char          control_url[1024];
    char          sdp_ip[32];
    int           sdp_port;
    int           sdp_ttl; // time to live
    RTSPTransport transport;
    int           sdp_payload_type;
} RTSPStream;

/**********************************************************
 * RTSP STREAM INFO。
 **********************************************************/
struct RTSPStreamInfo {
    char       metadata[32];
    char       default_ip[32]; // connection address
    int        ttl = 16;
    RTSPStream stream[8];
};

// typedef struct RTSPState {
//     int                     nb_rtsp_streams;
//     struct RTSPStream     **rtsp_streams;
//     enum RTSPClientState    state;
//     int64_t                 seek_timestamp;
//     int                     seq;
//     char                    session_id[512];
//     int                     timeout;
//     int64_t                 last_cmd_time;
//     enum RTSPTransport      transport;
//     enum RTSPLowerTransport lower_transport;
//     enum RTSPServerType     server_type;
//     char                    real_challenge[64];
//     char                    auth[128];
//     HTTPAuthState           auth_state;
//     char                    last_reply[2048];
//     void                   *cur_transport_priv;
//     int                     need_subscription;
//     char                    last_subscription[1024];
//     uint64_t                asf_pb_pos;
//     struct MpegTSContext   *ts;
//     int                     recvbuf_pos;
//     int                     recvbuf_len;
//     int                     nb_byes;
//     uint8_t                *recvbuf;
//     int                     lower_transport_mask;
//     uint64_t                packets;
//     struct pollfd          *p;
//     int                     max_p;
//     int                     initial_pause;
//     int                     rtp_muxer_flags;
//     int                     rtsp_flags;
//     int                     media_type_mask;
//     int                     rtp_port_min, rtp_port_max;
//     int                     initial_timeout;
//     int64_t                 stimeout;
//     int                     reordering_queue_size;
//     char                   *user_agent;
//     char                    default_lang[4];
//     int                     buffer_size;
//     int                     pkt_size;
//     char                   *localaddr;
// } RTSPState;
