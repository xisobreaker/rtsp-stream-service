#pragma once

#include "rtsp.h"
#include <string>

/**
 * 获取区间数据(如: 10087-10088)
 * @param min_ptr 左值
 * @param max_ptr 右值
 * @param msg
 */
void rtsp_parse_range(int *min_ptr, int *max_ptr, const std::string &msg);

/**
 * Range npt 解析
 */
void rtsp_parse_range_npt(const std::string &msg, int64_t *start, int64_t *end);

/**
 * RTSP transport 数据解析
 * @param ctx
 * @param reply
 */
void rtsp_parse_transport(RTSPContext *ctx, RTSPMessage *reply, const std::string &msg);

/**
 * Authorization 数据处理
 */
void http_auth_handle_header(HTTPAuthState *state, const std::string &key, const std::string &value);