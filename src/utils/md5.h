#ifndef MD5_H_H_H
#define MD5_H_H_H

#include <cstring>
#include <iomanip>
#include <stdlib.h>
#include <string>

bool md5_encrypt(unsigned char *dst, const unsigned char *src, int len);
bool md5_encrypt_hex(unsigned char *dst, const unsigned char *src, int len);

#endif
