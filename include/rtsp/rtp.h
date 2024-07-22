// =====================================================================================
//  Copyright (C) 2024 by Jiaxing Shao. All rights reserved
//
//  文 件 名:  rtp.h
//  作    者:  Jiaxing Shao, 980853650@qq.com
//  创建时间:  2024-07-09 14:18:52
//  描    述:
// =====================================================================================
#pragma once

typedef enum {
    RTCP_SDES_END   = 0,
    RTCP_SDES_CNAME = 1,
    RTCP_SDES_NAME  = 2,
    RTCP_SDES_EMAIL = 3,
    RTCP_SDES_PHONE = 4,
    RTCP_SDES_LOC   = 5,
    RTCP_SDES_TOOL  = 6,
    RTCP_SDES_NOTE  = 7,
    RTCP_SDES_PRIV  = 8
} rtcp_sdes_type_enum;
