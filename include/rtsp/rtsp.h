#pragma once

#include "http_auth.h"
#include "rtsp_def.h"
#include <arpa/inet.h>

#define RTSP_MAX_TRANSPORTS 8

/**********************************************************
 * RTSP Context。
 **********************************************************/
typedef struct {
    int                  seq;
    char                *user_agent;
    char                 session_id[512];
    char                 auth[128];
    HTTPAuthState        auth_state;
    RTSPControlTransport control_transport;
} RTSPContext;

/**********************************************************
 * RTSP Transport 元素。
 **********************************************************/
typedef struct RTSPTransportField {
    /** interleave ids, 如果是TCP方式传输数据，每个 TCP/RTSP 数据包头以魔数 '$'，
     * stream 长度和 stream ID。 如果 stream ID 是在 interleaved_min-max 的
     * 范围内, 则此数据包属于该流。 */
    int interleaved_min, interleaved_max;
    int port_min, port_max; // UDP 多播端口范围
    /** UDP 客户端端口; 本地计算机使用此范围内端口创建 UDP RTP（和RTCP）套接字，以接收 RTP/RTCP 数据。 */
    int              client_port_min, client_port_max;
    int              server_port_min, server_port_max; // UDP 单播端口范围
    int              ttl;
    int              mode_record;
    sockaddr_storage destination;
    char             source[INET6_ADDRSTRLEN + 1];

    RTSPTransport      transport;       // 数据包传输协议
    RTSPLowerTransport lower_transport; // 网络层传输协议
} RTSPTransportField;

/**********************************************************
 * RTSP Message。
 **********************************************************/
typedef struct {
    enum RTSPStatusCode status_code;            // rtsp 状态码
    int                 nb_transports;          // transports 项数量
    int64_t             range_start, range_end; // 服务将传输的流的时间范围
    int                 content_length;
    RTSPTransportField  transports[RTSP_MAX_TRANSPORTS];
    int                 seq;
    char                session_id[512];
    int                 timeout;
    char                location[4096];
    char                real_challenge[64];
    char                server[64];
    int                 notice;
    char                reason[256];
    char                content_type[64];
    char                stream_id[64];
} RTSPMessage;

/**********************************************************
 * RTSPState。
 **********************************************************/
typedef struct RTSPState {
    int                       nb_rtsp_streams;
    struct RTSPStream       **rtsp_streams;
    enum RTSPClientState      state;
    int64_t                   seek_timestamp;
    int                       seq;
    char                      session_id[512];
    int                       timeout;
    int64_t                   last_cmd_time;
    enum RTSPTransport        transport;
    enum RTSPLowerTransport   lower_transport;
    enum RTSPServerType       server_type;
    char                      real_challenge[64];
    char                      auth[128];
    HTTPAuthState             auth_state;
    char                      last_reply[2048];
    void                     *cur_transport_priv;
    int                       need_subscription;
    char                      last_subscription[1024];
    uint64_t                  asf_pb_pos;
    char                      control_uri[4096];
    struct MpegTSContext     *ts;
    int                       recvbuf_pos;
    int                       recvbuf_len;
    enum RTSPControlTransport control_transport;
    int                       nb_byes;
    uint8_t                  *recvbuf;
    int                       lower_transport_mask;
    uint64_t                  packets;
    struct pollfd            *p;
    int                       max_p;
    int                       get_parameter_supported;
    int                       initial_pause;
    int                       rtp_muxer_flags;
    int                       accept_dynamic_rate;
    int                       rtsp_flags;
    int                       media_type_mask;
    int                       rtp_port_min, rtp_port_max;
    int                       initial_timeout;
    int64_t                   stimeout;
    int                       reordering_queue_size;
    char                     *user_agent;
    char                      default_lang[4];
    int                       buffer_size;
    int                       pkt_size;
    char                     *localaddr;
} RTSPState;

/**
 * 命令数据编码
 * @param ctx
 * @param method
 * @param uri
 * @param headers
 */
std::string rtsp_method_encode(RTSPContext *ctx, const char *method, const char *uri, const char *headers);

/**
 * 获取区间数据(如: 10087-10088)
 * @param min_ptr 左值
 * @param max_ptr 右值
 * @param pp
 */
void rtsp_parse_range(int *min_ptr, int *max_ptr, const char **pp);

/**
 * 获取字符串，直到分割符
 */
void get_str_until_chars(char *buf, int buf_size, const char *sep, const char **pp);

/**
 * 获取字符串，跳过斜杠
 */
void get_str_skip_slash(char *buf, int buf_size, const char *sep, const char **pp);

/**
 * RTSP transport 数据解析
 * @param ctx
 * @param reply
 */
void rtsp_parse_transport(RTSPContext *ctx, RTSPMessage *reply, const char *p);

/**
 * 解析行数据
 * @param ctx
 * @param reply
 * @param message
 * @param rt
 * @param method
 */
void rtsp_parse_line(RTSPContext *ctx, RTSPMessage *reply, char *message, RTSPState *state, const char *method);

// Authorization 数据处理
void http_auth_handle_header(HTTPAuthState *state, const char *key, const char *value);