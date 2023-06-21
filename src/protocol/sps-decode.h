
#ifndef _SPS_DECODE_H_H_H
#define _SPS_DECODE_H_H_H

#include "rtsp-defines.h"

int          U(uint8_t *pBuf, uint32_t &nStartBit, uint32_t nBitSize);
unsigned int Ue(uint8_t *pBuf, uint32_t nLen, uint32_t &nStartBit);
int          Se(uint8_t *pBuf, uint32_t nLen, uint32_t &nStartBit);
void         de_emulation_prevention(uint8_t *pBuf, uint32_t &nLen);
int          h264_decode_sps(uint8_t *pBuf, uint32_t nLen, sps_data_t &sps);
int          h264_decode_pps(uint8_t *pBuf, uint32_t nLen, pps_data_t &pps);
void         h264_print_info(sps_data_t sps, pps_data_t pps);
void         h264_print_sps(sps_data_t sps);
void         h264_print_pps(pps_data_t pps);
#endif
