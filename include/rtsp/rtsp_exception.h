// =====================================================================================
//  Copyright (C) 2024 by Jiaxing Shao. All rights reserved
//
//  文 件 名:  rtsp_exception.h
//  作    者:  Jiaxing Shao, 980853650@qq.com
//  创建时间:  2024-07-09 14:19:31
//  描    述:
// =====================================================================================
#pragma once

#include <exception>
#include <string>

class RTSPException : public std::exception
{
public:
    RTSPException(std::string &&message)
    {
        m_errorMsg = message;
    };

public:
    virtual char const *what() const _GLIBCXX_TXN_SAFE_DYN _GLIBCXX_USE_NOEXCEPT
    {
        return m_errorMsg.c_str();
    }

private:
    std::string m_errorMsg;
};