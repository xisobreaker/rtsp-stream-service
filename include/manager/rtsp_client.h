// =====================================================================================
//  Copyright (C) 2024 by Jiaxing Shao. All rights reserved
//
//  文 件 名:  rtsp_client.h
//  作    者:  Jiaxing Shao, 980853650@qq.com
//  创建时间:  2024-07-09 14:23:54
//  描    述:
// =====================================================================================
#pragma once

#include "TcpClient.h"
#include "rtsp_url.h"
#include "structs.h"

class RtspClient
{
public:
    RtspClient(std::string rtspurl);
    ~RtspClient();

private:
    bool rtsp_parseline();
    bool rtsp_connect();

public:
    bool connect();

private:
    TcpClient   m_client;
    std::string m_rtspurl;

    RTSPUrlInfo *m_info;
    RTSPContext *m_ctx;
};
