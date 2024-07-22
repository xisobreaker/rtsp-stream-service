#include "rtpdec.h"
#include "rtp_utils.h"
#include <cstdint>
#include <cstring>

rtp_interleaved_frame_t rtp_interleaved_frame_parse(unsigned char *buffer)
{
    rtp_interleaved_frame_t frame;
    uint32_t                start_bit = 0;
    memset(&frame, 0, sizeof(rtp_interleaved_frame_t));
    frame.magic      = U(buffer, start_bit, 8);
    frame.channel_id = U(buffer, start_bit, 8);
    frame.length     = U(buffer, start_bit, 16);
    return frame;
}

rtp_hdr_t parse_rtp_header(unsigned char *buffer)
{
    rtp_hdr_t header;
    uint32_t  start_bit = 0;
    memset(&header, 0, sizeof(rtp_hdr_t));

    header.version = U(buffer, start_bit, 2);
    header.p       = U(buffer, start_bit, 1);
    header.x       = U(buffer, start_bit, 1);
    header.cc      = U(buffer, start_bit, 4);
    header.m       = U(buffer, start_bit, 1);
    header.pt      = U(buffer, start_bit, 7);
    header.seq     = U(buffer, start_bit, 16);
    header.ts      = U(buffer, start_bit, 32);
    header.ssrc    = U(buffer, start_bit, 32);
    for (int i = 0; i < header.cc; i++) {
        header.csrc[i] = U(buffer, start_bit, 32);
    }
    return header;
}