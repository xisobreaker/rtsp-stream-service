#include "urlcodec.h"
#include "memutils.h"

static inline int to_lower(int c)
{
    if (c >= 'A' && c <= 'Z') {
        c ^= 0x20;
    }
    return c;
}

static inline int is_digit(int c)
{
    return c >= '0' && c <= '9';
}

static inline int av_isxdigit(int c)
{
    c = to_lower(c);
    return (is_digit(c) || (c >= 'a' && c <= 'f'));
}

char *url_decode(const char *url, int decode_plus_sign)
{
    if (!url) {
        return nullptr;
    }

    int   url_len = strlen(url) + 1;
    char *dst = (char *)mem_malloc(url_len);
    if (!dst) {
        return NULL;
    }

    int pos = 0, idx = 0;
    while (pos < url_len) {
        char c = url[pos++];
        if (c == '%' && pos + 2 < url_len) {
            char c2 = url[pos++];
            char c3 = url[pos++];
            if (av_isxdigit(c2) && av_isxdigit(c3)) {
                c2 = to_lower(c2);
                c3 = to_lower(c3);

                if (c2 <= '9') {
                    c2 = c2 - '0';
                } else {
                    c2 = c2 - 'a' + 10;
                }

                if (c3 <= '9') {
                    c3 = c3 - '0';
                } else {
                    c3 = c3 - 'a' + 10;
                }
                dst[idx++] = 16 * c2 + c3;
            } else {
                dst[idx++] = c;
                dst[idx++] = c2;
                dst[idx++] = c3;
            }
        } else if (c == '+' && decode_plus_sign) {
            dst[idx++] = ' ';
        } else {
            dst[idx++] = c;
        }
    }
    return dst;
}
