#pragma once

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

/**
 * base64 编码
 * @param srcBuf
 * @param srcLen
 * @param dstBuf
 * @param dstMaxLen
 */
int base64_encode(const char *srcBuf, int srcLen, char *dstBuf, int dstMaxLen);

/**
 * base64 解码
 * @param srcBuf
 * @param srcLen
 * @param dstBuf
 * @param dstMaxLen
 */
int base64_decode(const char *srcBuf, int srcLen, char *dstBuf, int dstMaxLen);