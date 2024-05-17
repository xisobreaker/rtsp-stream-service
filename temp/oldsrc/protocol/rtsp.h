#pragma once

#include "httpauth.h"
#include "rtspdefs.h"
#include <memory>

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
 * 获取区间数据(如: 10087-10088)
 * @param min_ptr 左值
 * @param max_ptr 右值
 * @param pp
 */
void rtsp_parse_range(int *min_ptr, int *max_ptr, const char **pp);

/**
 * RTSP transport 数据解析
 */
void rtsp_parse_transport(RTSPContext *ctx, RTSPMessageHeader *reply, const char *p);

/**
 * 命令数据编码
 * @param ctx
 * @param method
 * @param uri
 * @param headers
 */
std::string rtsp_method_encode(RTSPContext *ctx, const char *method, const char *uri, const char *headers);

void rtsp_method_decode(RTSPContext *ctx, RTSPMessageHeader *reply, char *buf, RTSPState *rt, const char *method);

/**
 * 解析行数据
 * @param ctx
 * @param reply
 * @param buf
 * @param rt
 * @param method
 */
void rtsp_parse_line(RTSPContext *ctx, RTSPMessageHeader *reply, char *buf, RTSPState *rt, const char *method);

/**
 * 接收应答
 * @param fd socker 套接字
 * @param reply
 * @param content_ptr
 * @param method
 */
int rtsp_read_reply(int fd, RTSPMessageHeader *reply, unsigned char **content_ptr, const char *method);