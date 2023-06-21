#include "strutils.h"
#include "memutils.h"

#include <cstdarg>
#include <memory>

char **string_split(const char *msg, const char *sep, int &count)
{
    // int pos = 0, idx = 0;
    // while ((idx = msg.find(sep, pos)) != std::string::npos) {
    //     string str = msg.substr(pos, idx - pos);
    //     strQueue.push(str);
    //     pos = idx + sep.length();
    // }
    // return strQueue;
    return nullptr;
}

size_t string_lcpy(char *dst, const char *src, size_t maxlen)
{
    size_t len = 0;
    while (++len < maxlen && *src) {
        *dst++ = *src++;
    }
    if (len <= maxlen) {
        *dst = 0;
    }
    return len + strlen(src) - 1;
}

size_t string_lcat(char *dst, const char *src, size_t maxlen)
{
    size_t len = strlen(dst);
    if (maxlen <= len + 1) {
        return len + strlen(src);
    }
    return len + string_lcpy(dst + len, src, maxlen - len);
}

size_t snprint_lcatf(char *dst, size_t maxlen, const char *fmt, ...)
{
    size_t  len = strlen(dst);
    va_list vargs;

    va_start(vargs, fmt);
    char  *buf = dst + len;
    size_t size = maxlen > len ? maxlen - len : 0;
    len += vsnprintf(buf, size, fmt, vargs);
    va_end(vargs);

    return len;
}