#pragma once

#include "rtspdefs.h"
#include <memory>

/**
 * 解析视频 url 地址
 * @param info 解析后的信息
 * @param url  视频url地址
 */
void split_video_url(RTSPUrlInfo *info, const char *url);

/**
 * 获取字符串，直到分割符
 */
void get_str_until_chars(char *buf, int buf_size, const char *sep, const char **pp);

/**
 * 获取字符串，跳过斜杠
 */
void get_str_skip_slash(char *buf, int buf_size, const char *sep, const char **pp);