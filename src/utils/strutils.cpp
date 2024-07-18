#include "strutils.h"
#include "memutils.h"

#include <cstdarg>
#include <memory>
#include <vector>

size_t str_copy(char *dst, const char *src, size_t maxlen)
{
    size_t len = 0;
    while (++len < maxlen && *src) {
        *dst++ = *src++;
    }
    if (len <= maxlen) {
        *dst = 0;
    }
    return len - 1;
}

size_t str_lcat(char *dst, const char *src, size_t maxlen)
{
    size_t len = strlen(dst);
    if (maxlen <= len + 1) {
        return len + strlen(src);
    }
    return len + str_copy(dst + len, src, maxlen - len);
}

size_t snprint_lcatf(char *dst, size_t maxlen, const char *fmt, ...)
{
    size_t  len = strlen(dst);
    va_list vargs;

    va_start(vargs, fmt);
    char  *buf  = dst + len;
    size_t size = maxlen > len ? maxlen - len : 0;
    len += vsnprintf(buf, size, fmt, vargs);
    va_end(vargs);

    return len;
}

bool str_istart(const char *str, const char *prefix, const char **ptr)
{
    while (*prefix && ch_tolower(*prefix) == ch_tolower(*str)) {
        prefix++;
        str++;
    }
    if (!(*prefix) && ptr) {
        *ptr = str;
    }
    return !(*prefix);
}

int str_casecmp(const char *a, const char *b)
{
    uint8_t c1, c2;
    do {
        c1 = ch_tolower(*a++);
        c2 = ch_tolower(*b++);
    } while (c1 && c1 == c2);
    return c1 - c2;
}

int str_casencmp(const char *a, const char *b, size_t n)
{
    uint8_t c1, c2;
    if (n <= 0)
        return 0;
    do {
        c1 = ch_tolower(*a++);
        c2 = ch_tolower(*b++);
    } while (--n && c1 && c1 == c2);
    return c1 - c2;
}

std::string str_trim(const std::string &msg, const char ch)
{
    std::string message = msg;
    while (message.length() > 0) {
        if (message[0] != ch) {
            break;
        }
        message.erase(0, 1);
    }

    while (message.length() > 0) {
        if (message[message.length() - 1] != ch)
            break;
        message.erase(message.length() - 1, message.length());
    }
    return message;
}

std::vector<std::string> str_split(const std::string &msg, std::string sep, bool trimBlank)
{
    std::vector<std::string> vecStrs;

    int start = 0, pos = 0;
    while ((pos = msg.find(sep, start)) != std::string::npos) {
        if (start != pos) {
            std::string str = msg.substr(start, pos - start);
            if (trimBlank) {
                str = str_trim(str, ' ');
            }
            vecStrs.push_back(str);
        }
        start = pos + sep.length();
    }
    if (start != msg.length()) {
        std::string str = msg.substr(start);
        if (trimBlank) {
            str = str_trim(str, ' ');
        }
        vecStrs.push_back(str);
    }
    return vecStrs;
}

std::string str_cut_until_char(std::string &msg, std::string seps, bool keepSep)
{
    std::string message = std::move(msg);
    for (int i = 0; i < message.length(); i++) {
        if (seps.find(message.at(i)) != std::string::npos) {
            if (keepSep) {
                msg = message.substr(i);
            } else {
                msg = message.substr(i + 1);
            }
            return message.substr(0, i);
        }
    }
    return message;
}

bool str_starts_with(const std::string &str, std::string start)
{
    auto startLen = start.size();
    if (str.size() >= startLen) {
        std::string temp = str.substr(0, startLen);
        return temp.compare(start) == 0;
    }
    return false;
}

bool str_start_and_cut(std::string &msg, std::string start, bool skipSpace)
{
    if (!str_starts_with(msg, start)) {
        return false;
    }

    int startPos = start.length();
    if (skipSpace) {
        while (msg.at(startPos) == ' ') {
            startPos++;
        }
    }
    msg = msg.substr(startPos, msg.length());
    return true;
}

std::string str_format(const char *fmt, ...)
{
    char    buf[4096] = {0};
    va_list args;
    va_start(args, fmt);
    int n = vsnprintf(buf, 4096, fmt, args);
    va_end(args);
    return std::string(buf);
}

char *str_split_values(char *p, char sep, const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    char *temp = p;

    while (*p == sep)
        p++;

    while (*fmt) {
        char         **s, *tmp;
        int           *i;
        long long int *l;
        time_t        *t;

        switch (*fmt++) {
            case 's':
                s   = va_arg(va, char **);
                *s  = p;
                tmp = strchr(p, sep);
                if (tmp) {
                    while (*tmp == sep) {
                        *tmp++ = '\0';
                    }
                    p = tmp;
                } else {
                    p = &p[strlen(p)];
                }
                break;
            case 'l':
                l  = va_arg(va, long long int *);
                *l = strtoll(p, &tmp, 10);
                if (tmp == p) {
                    *p = 0;
                } else {
                    p = tmp;
                }
                break;
            case 'i':
                i  = va_arg(va, int *);
                *i = strtol(p, &tmp, 10);
                if (tmp == p) {
                    *p = 0;
                } else {
                    p = tmp;
                }
                break;
            case 't':
                t  = va_arg(va, time_t *);
                *t = strtol(p, &tmp, 10);
                if (tmp == p) {
                    *p = 0;
                } else {
                    p = tmp;
                    switch (*p) {
                        case 'd':
                            *t *= 86400;
                            *p++;
                            break;
                        case 'h':
                            *t *= 3600;
                            *p++;
                            break;
                        case 'm':
                            *t *= 60;
                            *p++;
                            break;
                    }
                }
                break;
        }
        while (*p == sep) {
            p++;
        }
    }
    va_end(va);
    return p;
}