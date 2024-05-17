#ifndef BASE64_H_H_H
#define BASE64_H_H_H

#include <cstring>
#include <sstream>
#include <string>

/**
 * base64 编码
 */
int base64_encode(const char *srcBuf, int srcLen, char *dstBuf, int dstMaxLen);

/**
 * base64 解码
 */
int base64_decode(const char *srcBuf, int srcLen, char *dstBuf, int dstMaxLen);

#endif
