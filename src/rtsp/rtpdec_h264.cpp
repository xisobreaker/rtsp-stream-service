#include "rtpdec_h264.h"
#include <cmath>
#include <cstring>
#include "rtp_utils.h"
#include "rtpdec.h"

rtp_nalu_hdr_t rtp_payload_parse_h264(unsigned char *buffer)
{
    rtp_nalu_hdr_t nalu_hdr;
    uint32_t       start_bit = 0;
    memset(&nalu_hdr, 0, sizeof(rtp_nalu_hdr_t));

    nalu_hdr.f    = U(buffer, start_bit, 1);
    nalu_hdr.nri  = U(buffer, start_bit, 2);
    nalu_hdr.type = U(buffer, start_bit, 5);
    return nalu_hdr;
}

int parse_nalu_stap(rtp_h264_nalu_t *nalu, unsigned char *buffer)
{
    int            nalu_size = (buffer[0] << 8) + buffer[1];
    rtp_nalu_hdr_t nalu_hdr  = rtp_payload_parse_h264(buffer + 2);
    nalu->data[0]            = 0x00;
    nalu->data[1]            = 0x00;
    nalu->data[2]            = 0x00;
    nalu->data[3]            = 0x01;
    memcpy(nalu->data + 4, buffer + 2, nalu_size);
    nalu->type       = nalu_hdr.type;
    nalu->size       = nalu_size + 4;
    nalu->union_type = (nalu_hdr.f << 7) + (nalu_hdr.nri << 5) + nalu_hdr.type;
    nalu->finish     = true;
    return nalu_size + 2;
}

int parse_nalu(rtp_h264_nalu_t *nalu, unsigned char *buffer, int length)
{
    int       pos    = 0;
    rtp_hdr_t header = parse_rtp_header(buffer + pos);
    pos += 12 + header.cc * 4;
    rtp_nalu_hdr_t nalu_hdr = rtp_payload_parse_h264(buffer + pos);
    pos += 1;
    if (nalu_hdr.type <= 23) {
        pos -= 1;
        nalu->data[0] = 0x00;
        nalu->data[1] = 0x00;
        nalu->data[2] = 0x00;
        nalu->data[3] = 0x01;
        memcpy(nalu->data + 4, buffer + pos, length - pos);
        nalu->type       = nalu_hdr.type;
        nalu->size       = length - pos + 4;
        nalu->union_type = (nalu_hdr.f << 7) + (nalu_hdr.nri << 5) + nalu_hdr.type;
        nalu->finish     = true;
        return 0;
    } else if (nalu_hdr.type == 24) {
        int n = parse_nalu_stap(nalu, buffer + pos);
        return pos + n;
    } else if (nalu_hdr.type == 25) {
        nalu->size   = 0;
        nalu->finish = false;
    } else if (nalu_hdr.type == 26) {
        nalu->size   = 0;
        nalu->finish = false;
    } else if (nalu_hdr.type == 27) {
        nalu->size   = 0;
        nalu->finish = false;
    } else if (nalu_hdr.type == 28) {
        int s    = (buffer[pos] >> 7) & 0x01;
        int e    = (buffer[pos] >> 6) & 0x01;
        int r    = (buffer[pos] >> 5) & 0x01; // 保留字节
        int type = buffer[pos] & 0x1F;
        if (s) {
            buffer[pos]   = nalu->union_type & 0xFF;
            nalu->data[0] = 0x00;
            nalu->data[1] = 0x00;
            nalu->data[2] = 0x00;
            nalu->data[3] = 0x01;
            memcpy(nalu->data + 4, buffer + pos, length - pos);
            nalu->type       = type;
            nalu->size       = length - pos + 4;
            nalu->union_type = (nalu_hdr.f << 7) + (nalu_hdr.nri << 5) + type;
            nalu->data[4]    = nalu->union_type & 0xFF;
            nalu->finish     = false;
        } else if (e) {
            if (nalu->size == 0)
                return 0;
            pos += 1;
            memcpy(nalu->data + nalu->size, buffer + pos, length - pos);
            nalu->size   = length - pos + nalu->size;
            nalu->finish = true;
        } else {
            if (nalu->size == 0)
                return 0;
            pos += 1;
            memcpy(nalu->data + nalu->size, buffer + pos, length - pos);
            nalu->size = length - pos + nalu->size;
        }
        return 0;
    } else if (nalu_hdr.type == 29) {
        nalu->size   = 0;
        nalu->finish = false;
    } else {
        printf("parse_nalu error, unknown message, %d.", nalu_hdr.type);
        nalu->size   = 0;
        nalu->finish = false;
    }
    return -1;
}

int h264_decode_sps(uint8_t *pBuf, uint32_t nLen, sps_data_t &sps)
{
    de_emulation_prevention(pBuf, nLen);
    uint32_t nStartBit = 0;
    if (pBuf[0] == 0x00 && pBuf[1] == 0x00 && pBuf[2] == 0x00 && pBuf[3] == 0x01) {
        nStartBit = 4 * 8;
    } else if (pBuf[0] == 0x00 && pBuf[1] == 0x00 && pBuf[2] == 0x01) {
        nStartBit = 3 * 8;
    } else {
        return -1;
    }

    int forbidden_zero_bit = U(pBuf, nStartBit, 1);
    int nal_ref_idc        = U(pBuf, nStartBit, 2);
    int nal_unit_type      = U(pBuf, nStartBit, 5);
    if (nal_unit_type != 7) {
        return -1;
    }

    sps.profile_idc          = U(pBuf, nStartBit, 8);
    sps.constraint_set0_flag = U(pBuf, nStartBit, 1);
    sps.constraint_set1_flag = U(pBuf, nStartBit, 1);
    sps.constraint_set2_flag = U(pBuf, nStartBit, 1);
    sps.constraint_set3_flag = U(pBuf, nStartBit, 1);
    sps.constraint_set4_flag = U(pBuf, nStartBit, 1);
    sps.constraint_set5_flag = U(pBuf, nStartBit, 1);
    sps.reserved_zero_2bits  = U(pBuf, nStartBit, 2);
    sps.level_id             = U(pBuf, nStartBit, 8);
    sps.seq_parameter_set_id = Ue(pBuf, nLen, nStartBit);
    if (sps.profile_idc == 100 || sps.profile_idc == 110 || sps.profile_idc == 122 || sps.profile_idc == 144) {
        sps.chroma_format_idc = Ue(pBuf, nLen, nStartBit);
        if (sps.chroma_format_idc == 3) {
            sps.residual_colour_transform_flag = U(pBuf, nStartBit, 1);
        }

        sps.bit_depth_luma_minus8                = Ue(pBuf, nLen, nStartBit);
        sps.bit_depth_chroma_minus8              = Ue(pBuf, nLen, nStartBit);
        sps.qpprime_y_zero_transform_bypass_flag = U(pBuf, nStartBit, 1);
        sps.seq_scaling_matrix_present_flag      = U(pBuf, nStartBit, 1);
        if (sps.seq_scaling_matrix_present_flag) {
            for (int i = 0; i < 8; i++) {
                sps.seq_scaling_list_present_flag[i] = U(pBuf, nStartBit, 1);
            }
        }
    }
    sps.log2_max_frame_num_minus4 = Ue(pBuf, nLen, nStartBit);
    sps.pic_order_cnt_type        = Ue(pBuf, nLen, nStartBit);
    if (sps.pic_order_cnt_type == 0) {
        sps.log2_max_pic_order_cnt_lsb_minus4 = Ue(pBuf, nLen, nStartBit);
    } else if (sps.pic_order_cnt_type == 1) {
        sps.delta_pic_order_always_zero_flag      = U(pBuf, nStartBit, 1);
        sps.offset_for_non_ref_pic                = Se(pBuf, nLen, nStartBit);
        sps.offset_for_top_to_bottom_field        = Se(pBuf, nLen, nStartBit);
        sps.num_ref_frames_in_pic_order_cnt_cycle = Ue(pBuf, nLen, nStartBit);
        for (int i = 0; i < sps.num_ref_frames_in_pic_order_cnt_cycle; i++) {
            sps.offset_for_ref_frame[i] = Se(pBuf, nLen, nStartBit);
        }
    }
    sps.num_ref_frames                       = Ue(pBuf, nLen, nStartBit);
    sps.gaps_in_frame_num_value_allowed_flag = U(pBuf, nStartBit, 1);
    sps.pic_width_in_mbs_minus1              = Ue(pBuf, nLen, nStartBit);
    sps.pic_height_in_map_units_minus1       = Ue(pBuf, nLen, nStartBit);
    sps.frame_mbs_only_flag                  = U(pBuf, nStartBit, 1);
    if (!sps.frame_mbs_only_flag) {
        sps.mb_adaptive_frame_field_flag = U(pBuf, nStartBit, 1);
    }
    sps.direct_8x8_inference_flag = U(pBuf, nStartBit, 1);
    sps.frame_cropping_flag       = U(pBuf, nStartBit, 1);
    if (sps.frame_cropping_flag) {
        sps.frame_crop_left_offset   = Ue(pBuf, nLen, nStartBit);
        sps.frame_crop_right_offset  = Ue(pBuf, nLen, nStartBit);
        sps.frame_crop_top_offset    = Ue(pBuf, nLen, nStartBit);
        sps.frame_crop_bottom_offset = Ue(pBuf, nLen, nStartBit);
    }
    sps.vui_parameter_present_flag = U(pBuf, nStartBit, 1);
    if (sps.vui_parameter_present_flag) {
        sps.aspect_ratio_info_present_flag = U(pBuf, nStartBit, 1);
        if (sps.aspect_ratio_info_present_flag) {
            sps.aspect_ratio_idc = U(pBuf, nStartBit, 8);
            if (sps.aspect_ratio_idc == 255) {
                sps.sar_width  = U(pBuf, nStartBit, 16);
                sps.sar_height = U(pBuf, nStartBit, 16);
            }
        }
        sps.overscan_info_present_flag = U(pBuf, nStartBit, 1);
        if (sps.overscan_info_present_flag) {
            sps.overscan_appropriate_flagu = U(pBuf, nStartBit, 1);
        }
        sps.video_signal_type_present_flag = U(pBuf, nStartBit, 1);
        if (sps.video_signal_type_present_flag) {
            sps.video_format                    = U(pBuf, nStartBit, 3);
            sps.video_full_range_flag           = U(pBuf, nStartBit, 1);
            sps.colour_description_present_flag = U(pBuf, nStartBit, 1);
            if (sps.colour_description_present_flag) {
                sps.colour_primaries         = U(pBuf, nStartBit, 8);
                sps.transfer_characteristics = U(pBuf, nStartBit, 8);
                sps.matrix_coefficients      = U(pBuf, nStartBit, 8);
            }
        }
        sps.chroma_location_info_present_flag = U(pBuf, nStartBit, 1);
        if (sps.chroma_location_info_present_flag) {
            sps.chroma_location_type_top_filed    = Ue(pBuf, nLen, nStartBit);
            sps.chroma_location_type_bottom_filed = Ue(pBuf, nLen, nStartBit);
        }
        sps.timing_info_present_flag = U(pBuf, nStartBit, 1);
        if (sps.timing_info_present_flag) {
            sps.num_units_in_tick = U(pBuf, nStartBit, 32);
            sps.time_scale        = U(pBuf, nStartBit, 32);
        }
    }
    return 0;
}

int h264_decode_pps(uint8_t *pBuf, uint32_t nLen, pps_data_t &pps)
{
    de_emulation_prevention(pBuf, nLen);
    uint32_t nStartBit = 0;
    if (pBuf[0] == 0x00 && pBuf[1] == 0x00 && pBuf[2] == 0x00 && pBuf[3] == 0x01) {
        nStartBit = 4 * 8;
    } else if (pBuf[0] == 0x00 && pBuf[1] == 0x00 && pBuf[2] == 0x01) {
        nStartBit = 3 * 8;
    } else {
        return -1;
    }

    int forbidden_zero_bit = U(pBuf, nStartBit, 1);
    int nal_ref_idc        = U(pBuf, nStartBit, 2);
    int nal_unit_type      = U(pBuf, nStartBit, 5);
    if (nal_unit_type != 8) {
        return -1;
    }
    return 0;
}

void h264_print_info(sps_data_t sps, pps_data_t pps)
{
    int width  = 0;
    int height = 0;
    int fps    = 0;

    width  = (sps.pic_width_in_mbs_minus1 + 1) * 16;
    height = (sps.pic_height_in_map_units_minus1 + 1) * 16;
    fps    = sps.time_scale / (2 * sps.num_units_in_tick);

    printf("*********************************** VIDEO INFO *************************************");
    printf("Width     : %d", width);
    printf("Height    : %d", width);
    printf("FPS       : %d", fps);
    printf("*************************************** END ****************************************");
}

void h264_print_sps(sps_data_t sps)
{
    printf("*********************************** SPS *************************************");
    printf("profile_idc                              : %d", sps.profile_idc);
    printf("constraint_set0_flag                     : %d", sps.constraint_set0_flag);
    printf("constraint_set1_flag                     : %d", sps.constraint_set1_flag);
    printf("constraint_set2_flag                     : %d", sps.constraint_set2_flag);
    printf("constraint_set3_flag                     : %d", sps.constraint_set3_flag);
    printf("constraint_set4_flag                     : %d", sps.constraint_set4_flag);
    printf("constraint_set5_flag                     : %d", sps.constraint_set5_flag);
    printf("reserved_zero_2bits                      : %d", sps.reserved_zero_2bits);
    printf("level_id                                 : %d", sps.level_id);
    printf("seq_parameter_set_id                     : %d", sps.seq_parameter_set_id);
    if (sps.profile_idc == 100 || sps.profile_idc == 110 || sps.profile_idc == 122 || sps.profile_idc == 144) {
        printf("chroma_format_idc                    : %d", sps.chroma_format_idc);
        if (sps.chroma_format_idc == 3)
            printf("        residual_colour_transform_flag: %d", sps.residual_colour_transform_flag);
        printf("    bit_depth_luma_minus8                : %d", sps.bit_depth_luma_minus8);
        printf("    bit_depth_chroma_minus8              : %d", sps.bit_depth_chroma_minus8);
        printf("    qpprime_y_zero_transform_bypass_flag : %d", sps.qpprime_y_zero_transform_bypass_flag);
        printf("    seq_scaling_matrix_present_flag      : %d", sps.seq_scaling_matrix_present_flag);
        if (sps.seq_scaling_matrix_present_flag)
            for (int i = 0; i < 8; i++)
                printf("        seq_scaling_list_present_flag[%d]: %d", i, sps.seq_scaling_list_present_flag[i]);
    }
    printf("log2_max_frame_num_minus4                : %d", sps.log2_max_frame_num_minus4);
    printf("pic_order_cnt_type                       : %d", sps.pic_order_cnt_type);
    if (sps.pic_order_cnt_type == 0) {
        printf("    log2_max_pic_order_cnt_lsb_minus4    : %d", sps.log2_max_pic_order_cnt_lsb_minus4);
    } else if (sps.pic_order_cnt_type == 1) {
        printf("    delta_pic_order_always_zero_flag     : %d", sps.delta_pic_order_always_zero_flag);
        printf("    offset_for_non_ref_pic               : %d", sps.offset_for_non_ref_pic);
        printf("    offset_for_top_to_bottom_field       : %d", sps.offset_for_top_to_bottom_field);
        printf("    num_ref_frames_in_pic_order_cnt_cycle: %d", sps.num_ref_frames_in_pic_order_cnt_cycle);
        for (int i = 0; i < sps.num_ref_frames_in_pic_order_cnt_cycle; i++)
            printf("    offset_for_ref_frame[%d]             : %d", i, sps.offset_for_ref_frame[i]);
    }
    printf("num_ref_frames                           : %d", sps.num_ref_frames);
    printf("gaps_in_frame_num_value_allowed_flag     : %d", sps.gaps_in_frame_num_value_allowed_flag);
    printf("pic_width_in_mbs_minus1                  : %d", sps.pic_width_in_mbs_minus1);
    printf("pic_height_in_map_units_minus1           : %d", sps.pic_height_in_map_units_minus1);
    printf("frame_mbs_only_flag                      : %d", sps.frame_mbs_only_flag);
    if (sps.frame_mbs_only_flag)
        printf("    mb_adaptive_frame_field_flag         : %d", sps.mb_adaptive_frame_field_flag);
    printf("direct_8x8_inference_flag                : %d", sps.direct_8x8_inference_flag);
    printf("frame_cropping_flag                      : %d", sps.frame_cropping_flag);
    if (sps.frame_cropping_flag) {
        printf("    frame_crop_left_offset               : %d", sps.frame_crop_left_offset);
        printf("    frame_crop_right_offset              : %d", sps.frame_crop_right_offset);
        printf("    frame_crop_top_offset                : %d", sps.frame_crop_top_offset);
        printf("    frame_crop_bottom_offset             : %d", sps.frame_crop_bottom_offset);
    }
    printf("vui_parameter_present_flag               : %d", sps.vui_parameter_present_flag);
    if (sps.vui_parameter_present_flag) {
        printf("    aspect_ratio_info_present_flag       : %d", sps.aspect_ratio_info_present_flag);
        if (sps.aspect_ratio_info_present_flag) {
            printf("        aspect_ratio_idc                 : %d", sps.aspect_ratio_idc);
            if (sps.aspect_ratio_idc) {
                printf("            sar_width                    : %d", sps.sar_width);
                printf("            sar_height                   : %d", sps.sar_height);
            }
        }
        printf("    overscan_info_present_flag           : %d", sps.overscan_info_present_flag);
        if (sps.overscan_info_present_flag)
            printf("        overscan_appropriate_flagu       : %d", sps.overscan_appropriate_flagu);
        printf("    video_signal_type_present_flag       : %d", sps.video_signal_type_present_flag);
        if (sps.video_signal_type_present_flag) {
            printf("        video_format                     : %d", sps.video_format);
            printf("        video_full_range_flag            : %d", sps.video_full_range_flag);
            printf("        colour_description_present_flag  : %d", sps.colour_description_present_flag);
            if (sps.colour_description_present_flag) {
                printf("            colour_primaries             : %d", sps.colour_primaries);
                printf("            transfer_characteristics     : %d", sps.transfer_characteristics);
                printf("            matrix_coefficients          : %d", sps.matrix_coefficients);
            }
        }
        printf("    chroma_location_info_present_flag    : %d", sps.chroma_location_info_present_flag);
        if (sps.chroma_location_info_present_flag) {
            printf("        chroma_location_type_top_filed   : %d", sps.chroma_location_type_top_filed);
            printf("        chroma_location_type_bottom_filed: %d", sps.chroma_location_type_bottom_filed);
        }
        printf("    timing_info_present_flag             : %d", sps.timing_info_present_flag);
        if (sps.timing_info_present_flag) {
            printf("        num_units_in_tick                : %d", sps.num_units_in_tick);
            printf("        time_scale                       : %d", sps.time_scale);
        }
    }
    printf("*********************************** END *************************************");
}

void h264_print_pps(pps_data_t pps)
{
}
