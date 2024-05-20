#ifndef URLCODEC_H_H_H
#define URLCODEC_H_H_H

#include <cstring>

/**
 * url 解析
 * @param url 网络地址
 * @param decode_plus_sign 是否解码 + 符号
 * @return 解析完后的 url 地址
 */
char *url_decode(const char *url, int decode_plus_sign);

#endif