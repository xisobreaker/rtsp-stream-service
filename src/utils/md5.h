#ifndef MD5_H_H_H
#define MD5_H_H_H

#include <cstring>
#include <iomanip>
#include <stdlib.h>
#include <string>

/**
 * md5 加密
 * @param dst 加密后的hex数据
 * @param src 要加密的数据
 * @param len 要加密的数据长度
 * @return 函数执行结果
 */
bool md5_encrypt(unsigned char *dst, const unsigned char *src, int len);

/**
 * md5 加密
 * @param dst 加密后的hex数据，转为字符串
 * @param src 要加密的数据
 * @param len 要加密的数据长度
 * @return 函数执行结果
 */
bool md5_encrypt_hex(unsigned char *dst, const unsigned char *src, int len);

#endif
