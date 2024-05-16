#ifndef _RTP_DECODE_H_H_H
#define _RTP_DECODE_H_H_H

typedef enum {
    RTCP_SR = 200,
    RTCP_RR = 201,
    RTCP_SDES = 202,
    RTCP_BYE = 203,
    RTCP_APP = 204
} rtcp_type_enum;

typedef enum {
    RTCP_SDES_END = 0,
    RTCP_SDES_CNAME = 1,
    RTCP_SDES_NAME = 2,
    RTCP_SDES_EMAIL = 3,
    RTCP_SDES_PHONE = 4,
    RTCP_SDES_LOC = 5,
    RTCP_SDES_TOOL = 6,
    RTCP_SDES_NOTE = 7,
    RTCP_SDES_PRIV = 8
} rtcp_sdes_type_enum;

typedef enum {
    RTP_PAYLOAD_STAP_A = 24,
    RTP_PAYLOAD_STAP_B = 25,
    RTP_PAYLOAD_MTAP16 = 26,
    RTP_PAYLOAD_MTAP24 = 27,
    RTP_PAYLOAD_FU_A = 28,
    RTP_PAYLOAD_FU_B = 29
} rtp_payload_h264_type_enum;

typedef struct {
    int magic;      /* 固定0x24 */
    int channel_id; /* 0x00 video rtp, 0x01 video rtcp, 0x02 audio rtp, 0x03 audio rtcp */
    int length;     /* 长度 */
} rtp_interleaved_frame_t;

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

/**********************************************************
 * RTP nalu header
 **********************************************************/
typedef struct {
    int f;
    int nri;
    int type;
} rtp_nalu_hdr_t;

/**********************************************************
 * RTP h264 nalu
 **********************************************************/
typedef struct {
    int  finish;
    int  type;
    int  union_type;
    int  size;
    char data[0];
} rtp_h264_nalu_t;

rtp_interleaved_frame_t parse_rtp_frame(unsigned char *buffer);
rtp_hdr_t               parse_rtp_header(unsigned char *buffer);
rtp_nalu_hdr_t          parse_rtp_h264_format(unsigned char *buffer);

int parse_nalu_stap(rtp_h264_nalu_t *nalu, unsigned char *buffer);
int parse_nalu(rtp_h264_nalu_t *nalu, unsigned char *buffer, int length);

#endif
