#pragma once

#include "httpauth.h"
#include "rtspdefs.h"
#include <memory>

typedef struct {
    int                  seq;
    char                *user_agent;
    char                 session_id[512];
    char                 auth[128];
    HTTPAuthState        auth_state;
    RTSPControlTransport control_transport;
} RTSPContext;

typedef struct RTSPState {
    /** number of items in the 'rtsp_streams' variable */
    int nb_rtsp_streams;

    struct RTSPStream **rtsp_streams; /**< streams in this session */

    /** indicator of whether we are currently receiving data from the
     * server. Basically this isn't more than a simple cache of the
     * last PLAY/PAUSE command sent to the server, to make sure we don't
     * send 2x the same unexpectedly or commands in the wrong state. */
    enum RTSPClientState state;

    /** the seek value requested when calling av_seek_frame(). This value
     * is subsequently used as part of the "Range" parameter when emitting
     * the RTSP PLAY command. If we are currently playing, this command is
     * called instantly. If we are currently paused, this command is called
     * whenever we resume playback. Either way, the value is only used once,
     * see rtsp_read_play() and rtsp_read_seek(). */
    int64_t seek_timestamp;

    int seq; /**< RTSP command sequence number */

    /** copy of RTSPMessageHeader->session_id, i.e. the server-provided session
     * identifier that the client should re-transmit in each RTSP command */
    char session_id[512];

    /** copy of RTSPMessageHeader->timeout, i.e. the time (in seconds) that
     * the server will go without traffic on the RTSP/TCP line before it
     * closes the connection. */
    int timeout;

    /** timestamp of the last RTSP command that we sent to the RTSP server.
     * This is used to calculate when to send dummy commands to keep the
     * connection alive, in conjunction with timeout. */
    int64_t last_cmd_time;

    /** the negotiated data/packet transport protocol; e.g. RTP or RDT */
    enum RTSPTransport transport;

    /** the negotiated network layer transport protocol; e.g. TCP or UDP
     * uni-/multicast */
    enum RTSPLowerTransport lower_transport;

    /** brand of server that we're talking to; e.g. WMS, REAL or other.
     * Detected based on the value of RTSPMessageHeader->server or the presence
     * of RTSPMessageHeader->real_challenge */
    enum RTSPServerType server_type;

    /** the "RealChallenge1:" field from the server */
    char real_challenge[64];

    /** plaintext authorization line (username:password) */
    char auth[128];

    /** authentication state */
    HTTPAuthState auth_state;

    /** The last reply of the server to a RTSP command */
    char last_reply[2048]; /* XXX: allocate ? */

    /** RTSPStream->transport_priv of the last stream that we read a
     * packet from */
    void *cur_transport_priv;

    /** The following are used for Real stream selection */
    //@{
    /** whether we need to send a "SET_PARAMETER Subscribe:" command */
    int need_subscription;

    /** the last value of the "SET_PARAMETER Subscribe:" RTSP command.
     * this is used to send the same "Unsubscribe:" if stream setup changed,
     * before sending a new "Subscribe:" command. */
    char last_subscription[1024];
    //@}

    /** cache for position of the asf demuxer, since we load a new
     * data packet in the bytecontext for each incoming RTSP packet. */
    uint64_t asf_pb_pos;
    //@}

    /** some MS RTSP streams contain a URL in the SDP that we need to use
     * for all subsequent RTSP requests, rather than the input URI; in
     * other cases, this is a copy of AVFormatContext->filename. */
    char control_uri[4096];

    /** The following are used for parsing raw mpegts in udp */
    //@{
    struct MpegTSContext *ts;
    int                   recvbuf_pos;
    int                   recvbuf_len;
    //@}

    /** RTSP transport mode, such as plain or tunneled. */
    enum RTSPControlTransport control_transport;

    /* Number of RTCP BYE packets the RTSP session has received.
     * An EOF is propagated back if nb_byes == nb_streams.
     * This is reset after a seek. */
    int nb_byes;

    /** Reusable buffer for receiving packets */
    uint8_t *recvbuf;

    /**
     * A mask with all requested transport methods
     */
    int lower_transport_mask;

    /**
     * The number of returned packets
     */
    uint64_t packets;

    /**
     * Polling array for udp
     */
    struct pollfd *p;
    int            max_p;

    /**
     * Whether the server supports the GET_PARAMETER method.
     */
    int get_parameter_supported;

    /**
     * Do not begin to play the stream immediately.
     */
    int initial_pause;

    /**
     * Option flags for the chained RTP muxer.
     */
    int rtp_muxer_flags;

    /** Whether the server accepts the x-Dynamic-Rate header */
    int accept_dynamic_rate;

    /**
     * Various option flags for the RTSP muxer/demuxer.
     */
    int rtsp_flags;

    /**
     * Mask of all requested media types
     */
    int media_type_mask;

    /**
     * Minimum and maximum local UDP ports.
     */
    int rtp_port_min, rtp_port_max;

    /**
     * Timeout to wait for incoming connections.
     */
    int initial_timeout;

    /**
     * timeout of socket i/o operations.
     */
    int64_t stimeout;

    /**
     * Size of RTP packet reordering queue.
     */
    int reordering_queue_size;

    /**
     * User-Agent string
     */
    char *user_agent;

    char  default_lang[4];
    int   buffer_size;
    int   pkt_size;
    char *localaddr;
} RTSPState;

// 获取字符串，直到分割符
void get_str_until_chars(char *buf, int buf_size, const char *sep, const char **pp);

// 获取字符串，跳过斜杠
void get_str_skip_slash(char *buf, int buf_size, const char *sep, const char **pp);

// 获取区间数据
void rtsp_parse_range(int *min_ptr, int *max_ptr, const char **pp);

// 发送命令
int rtsp_send_cmd_content(int fd, RTSPContext *ctx, const char *method, const char *uri, const char *headers);

// 命令数据编码
std::shared_ptr<char> rtsp_method_encode(RTSPContext *ctx, const char *method, const char *uri, const char *headers);

void rtsp_parse_transport(RTSPContext *ctx, RTSPMessageHeader *reply, const char *p);

// 解析行数据
void rtsp_parse_line(RTSPContext *ctx, RTSPMessageHeader *reply, char *buf, RTSPState *rt, const char *method);