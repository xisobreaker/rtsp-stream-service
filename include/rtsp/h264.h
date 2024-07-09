// =====================================================================================
//  Copyright (C) 2024 by Jiaxing Shao. All rights reserved
//
//  文 件 名:  h264.h
//  作    者:  Jiaxing Shao, 980853650@qq.com
//  创建时间:  2024-07-09 14:19:11
//  描    述:
// =====================================================================================
#ifndef H264_DECODE_H_H_H
#define H264_DECODE_H_H_H

#include <iomanip>

/**********************************************************
 * SPS 数据。
 **********************************************************/
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

    int pic_order_cnt_type;
    int log2_max_pic_order_cnt_lsb_minus4;
    int delta_pic_order_always_zero_flag;
    int offset_for_non_ref_pic;
    int offset_for_top_to_bottom_field;
    int num_ref_frames_in_pic_order_cnt_cycle;
    int offset_for_ref_frame[256];
    int num_ref_frames;
    int gaps_in_frame_num_value_allowed_flag;
    int pic_width_in_mbs_minus1;
    int pic_height_in_map_units_minus1;

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

/**********************************************************
 * PPS 数据。
 **********************************************************/
typedef struct {
} pps_data_t;

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
