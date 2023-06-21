#ifndef STRUTILS_H_H_H
#define STRUTILS_H_H_H

#include <cstring>

size_t string_lcpy(char *dst, const char *src, size_t maxlen);
size_t string_lcat(char *dst, const char *src, size_t maxlen);
size_t snprint_lcatf(char *dst, size_t maxlen, const char *fmt, ...);

#endif