#pragma once

#include "rtsp-constants.h"
#include <arpa/inet.h>
#include <iomanip>

/**
 * This describes a single item in the "Transport:" line of one stream as
 * negotiated by the SETUP RTSP command. Multiple transports are comma-
 * separated ("Transport: x-read-rdt/tcp;interleaved=0-1,rtp/avp/udp;
 * client_port=1000-1001;server_port=1800-1801") and described in separate
 * RTSPTransportFields.
 */
typedef struct RTSPTransportField {
    /** interleave ids, 如果是TCP方式传输数据，每个 TCP/RTSP 数据包头以魔数 '$'，
     * stream 长度和 stream ID。 如果 stream ID 是在 interleaved_min-max 的
     * 范围内, 则此数据包属于该流。 */
    int interleaved_min, interleaved_max;

    /** UDP 多播端口范围；连接此端口以接收 UDP 的多播数据。 */
    int port_min, port_max;

    /** UDP 客户端端口; 本地计算机使用此范围内端口创建 UDP RTP（和RTCP）套接字，以接收 RTP/RTCP 数据。 */
    int client_port_min, client_port_max;

    /** UDP 单播端口范围; the ports to which we should connect
     * to receive unicast UDP RTP/RTCP data. */
    int server_port_min, server_port_max;

    /** time-to-live value (required for multicast); the amount of HOPs that
     * packets will be allowed to make before being discarded. */
    int ttl;

    /** transport set to record data */
    int mode_record;

    struct sockaddr_storage destination;                  /**< destination IP address */
    char                    source[INET6_ADDRSTRLEN + 1]; /**< source IP address */

    /** 数据包传输协议; 如： RTP 或 RDT */
    enum RTSPTransport transport;

    /** 网络层传输协议; 如： TCP 或 UDP 单播/多播 */
    enum RTSPLowerTransport lower_transport;
} RTSPTransportField;

/**
 * 描述服务器对每个RTSP命令的响应。
 */
typedef struct RTSPMessageHeader {
    /** length of the data following this header */
    int content_length;

    enum RTSPStatusCode status_code; /**< response code from server */

    /** number of items in the 'transports' variable below */
    int nb_transports;

    /** Time range of the streams that the server will stream. In
     * AV_TIME_BASE unit, AV_NOPTS_VALUE if not used */
    int64_t range_start, range_end;

    /** describes the complete "Transport:" line of the server in response
     * to a SETUP RTSP command by the client */
    RTSPTransportField transports[RTSP_MAX_TRANSPORTS];

    int seq; /**< sequence number */

    /** the "Session:" field. This value is initially set by the server and
     * should be re-transmitted by the client in every RTSP command. */
    char session_id[512];

    /** the "Location:" field. This value is used to handle redirection.
     */
    char location[4096];

    /** the "RealChallenge1:" field from the server */
    char real_challenge[64];

    /** the "Server: field, which can be used to identify some special-case
     * servers that are not 100% standards-compliant. We use this to identify
     * Windows Media Server, which has a value "WMServer/v.e.r.sion", where
     * version is a sequence of digits (e.g. 9.0.0.3372). Helix/Real servers
     * use something like "Helix [..] Server Version v.e.r.sion (platform)
     * (RealServer compatible)" or "RealServer Version v.e.r.sion (platform)",
     * where platform is the output of $uname -msr | sed 's/ /-/g'. */
    char server[64];

    /** The "timeout" comes as part of the server response to the "SETUP"
     * command, in the "Session: <xyz>[;timeout=<value>]" line. It is the
     * time, in seconds, that the server will go without traffic over the
     * RTSP/TCP connection before it closes the connection. To prevent
     * this, sent dummy requests (e.g. OPTIONS) with intervals smaller
     * than this value. */
    int timeout;

    /** The "Notice" or "X-Notice" field value. See
     * http://tools.ietf.org/html/draft-stiemerling-rtsp-announce-00
     * for a complete list of supported values. */
    int notice;

    /** The "reason" is meant to specify better the meaning of the error code
     * returned
     */
    char reason[256];

    /**
     * Content type header
     */
    char content_type[64];

    /**
     * SAT>IP com.ses.streamID header
     */
    char stream_id[64];
} RTSPMessageHeader;

/**
 * 摘要认证参数结构体。
 */
typedef struct DigestParams {
    char nonce[300];
} DigestParams;

/**
 * HTTP 身份验证结构体
 */
typedef struct HTTPAuthState {
    // 身份验证类型
    HTTPAuthType auth_type;
    char         realm[200];
    DigestParams digest_params;
} HTTPAuthState;

typedef struct {
    int                       seq;
    char                     *user_agent;
    char                      session_id[512];
    char                      auth[128];
    HTTPAuthState             auth_state;
    enum RTSPControlTransport control_transport;
} RTSPContext;

typedef struct {
    int profile_idc;
    int constraint_set0_flag;
    int constraint_set1_flag;
    int constraint_set2_flag;
    int constraint_set3_flag;
    int constraint_set4_flag;
    int constraint_set5_flag;
    int reserved_zero_2bits;
    int level_id;
    int seq_parameter_set_id;
    int chroma_format_idc;
    int residual_colour_transform_flag;
    int bit_depth_luma_minus8;
    int bit_depth_chroma_minus8;
    int qpprime_y_zero_transform_bypass_flag;
    int seq_scaling_matrix_present_flag;
    int seq_scaling_list_present_flag[8];
    int log2_max_frame_num_minus4;

    int  pic_order_cnt_type;
    int  log2_max_pic_order_cnt_lsb_minus4;
    int  delta_pic_order_always_zero_flag;
    int  offset_for_non_ref_pic;
    int  offset_for_top_to_bottom_field;
    int  num_ref_frames_in_pic_order_cnt_cycle;
    int *offset_for_ref_frame;
    int  num_ref_frames;
    int  gaps_in_frame_num_value_allowed_flag;
    int  pic_width_in_mbs_minus1;
    int  pic_height_in_map_units_minus1;

    int frame_mbs_only_flag;
    int mb_adaptive_frame_field_flag;
    int direct_8x8_inference_flag;

    int frame_cropping_flag;
    int frame_crop_left_offset;
    int frame_crop_right_offset;
    int frame_crop_top_offset;
    int frame_crop_bottom_offset;

    int vui_parameter_present_flag;
    int aspect_ratio_info_present_flag;
    int aspect_ratio_idc;
    int sar_width;
    int sar_height;
    int overscan_info_present_flag;
    int overscan_appropriate_flagu;
    int video_signal_type_present_flag;
    int video_format;
    int video_full_range_flag;
    int colour_description_present_flag;
    int colour_primaries;
    int transfer_characteristics;
    int matrix_coefficients;
    int chroma_location_info_present_flag;
    int chroma_location_type_top_filed;
    int chroma_location_type_bottom_filed;
    int timing_info_present_flag;
    int num_units_in_tick;
    int time_scale;
    int fixed_frame_rate_flag;
    int nal_hrd_parameters_present_flag;
    int val_hrd_parameters_present_flag;
    int low_delay_hrd_flag;
    int pic_struct_present_flag;
    int bitstream_restriction_flag;
    int motion_vectors_over_pic_boundaries_flag;
    int max_bytes_per_pic_denom;
    int max_bytes_per_mb_denom;
    int log2_max_mv_length_vertical;
    int log2_max_mv_length_horizontal;
    int num_reorder_frames;
    int max_dec_frame_buffering;
} sps_data_t;

typedef struct {
} pps_data_t;

typedef struct {
    char     proto[16];
    char     authorization[128];
    char     hostname[16];
    uint16_t port;
    char     path[512];
} RTSPUrlInfo;