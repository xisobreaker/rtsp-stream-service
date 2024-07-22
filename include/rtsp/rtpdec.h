#pragma once

/**********************************************************
 * RTCP消息类型。
 **********************************************************/
typedef enum {
    RTCP_FIR  = 192,
    RTCP_SR   = 200,
    RTCP_RR   = 201,
    RTCP_SDES = 202,
    RTCP_BYE  = 203,
    RTCP_APP  = 204
} rtcp_type_enum;

/**********************************************************
 * RTP 消息头。
 **********************************************************/
typedef struct {
    unsigned int version;  /*  protocol version */
    unsigned int p;        /*  padding flag */
    unsigned int x;        /*  header extension flag */
    unsigned int cc;       /*  CSRC count */
    unsigned int m;        /*  marker bit */
    unsigned int pt;       /*  payload type */
    unsigned int seq;      /*  sequence number */
    unsigned int ts;       /*  timestamp */
    unsigned int ssrc;     /*  synchronization source */
    unsigned int csrc[15]; /*  optional CSRC list */
} rtp_hdr_t;

typedef struct {
    int magic;      /* 固定0x24 */
    int channel_id; /* 0x00 video rtp, 0x01 video rtcp, 0x02 audio rtp, 0x03 audio rtcp */
    int length;     /* 长度 */
} rtp_interleaved_frame_t;

rtp_interleaved_frame_t rtp_interleaved_frame_parse(unsigned char *buffer);
rtp_hdr_t               parse_rtp_header(unsigned char *buffer);