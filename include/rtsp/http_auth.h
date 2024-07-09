// =====================================================================================
//  Copyright (C) 2024 by Jiaxing Shao. All rights reserved
//
//  文 件 名:  http_auth.h
//  作    者:  Jiaxing Shao, 980853650@qq.com
//  创建时间:  2024-07-09 14:19:25
//  描    述:
// =====================================================================================
#pragma once

#include <string>

/**********************************************************
 * 身份验证类型，由弱到强。
 **********************************************************/
enum HTTPAuthType {
    HTTP_AUTH_NONE = 0, // 无身份校验
    HTTP_AUTH_BASIC,    // HTTP 1.0 基本认证 RFC 1945 (及 RFC 2617)
    HTTP_AUTH_DIGEST,   // HTTP 1.1 摘要认证 RFC 2617
};

/**********************************************************
 * 摘要认证参数结构体。
 **********************************************************/
struct DigestParams {
    char nonce[300];
};

/**********************************************************
 * HTTP 身份验证结构体。
 **********************************************************/
struct HTTPAuthState {
    HTTPAuthType auth_type;
    char         realm[200];
    DigestParams digest_params;
};

/**
 * 生成 basic 加密串
 * @param auth 用户密码信息(username:password)
 */
std::string make_basic_auth(const char *auth);

/**
 * 生成 digest 加密串
 * @param state
 * @param username 用户名
 * @param password 密码
 * @param uri      uri 地址
 * @param method   RTSP 命令
 */
std::string make_digest_auth(HTTPAuthState *state, const char *username, const char *password, const char *uri, const char *method);

/**
 * 返回生成的 Authorization 数据
 * @param state
 * @param auth
 * @param uri
 * @param method
 */
std::string http_auth_create_response(HTTPAuthState *state, const char *auth, const char *uri, const char *method);
