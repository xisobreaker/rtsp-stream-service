#ifndef STRUTILS_H_H_H
#define STRUTILS_H_H_H

#include <cstring>
#include <string>
#include <vector>

/**
 * 字母转大写
 * @param c 要转换的字符
 */
static inline char ch_toupper(char c)
{
    if (c >= 'a' && c <= 'z')
        c ^= 0x20;
    return c;
}

/**
 * 字母转小写
 * @param c 要转换的字符
 */
static inline char ch_tolower(char c)
{
    if (c >= 'A' && c <= 'Z')
        c ^= 0x20;
    return c;
}

/**
 * 字符串安全拷贝，返回字符串拷贝长度
 * @param dst
 * @param maxlen (aka unsigned long)
 * @param fmt
 */
size_t str_copy(char *dst, const char *src, size_t maxlen);

/**********************************************************
 * 字符串追加，返回字符串追加后的总长度
 **********************************************************/
size_t str_lcat(char *dst, const char *src, size_t maxlen);

/**********************************************************
 * 字符串格式化追加拷贝，返回字符串追加后的总长度
 **********************************************************/
size_t snprint_lcatf(char *dst, size_t maxlen, const char *fmt, ...);

/**********************************************************
 * 字符串前缀是否匹配
 **********************************************************/
bool str_istart(const char *str, const char *prefix, const char **ptr);

/**********************************************************
 * 字符串比较（忽略大小写）
 * @return 字符串相同则返回 0
 **********************************************************/
int str_casecmp(const char *a, const char *b);

/**********************************************************
 * 字符串指定长度比较（忽略大小写）
 **********************************************************/
int str_casencmp(const char *a, const char *b, size_t n);

/**********************************************************
 * 字符串指定长度比较（忽略大小写）
 **********************************************************/
std::string str_format(const char *fmt, ...);

char *str_split_values(char *p, char sep, const char *fmt, ...);

/**
 * 字符串修剪
 * @param msg 原始数据
 * @param ch 要修剪的字符
 */
std::string str_trim(const std::string &msg, const char ch);

/**
 * 字符串分割
 * @param msg
 * @param sep
 * @param trimBlank 是否修剪左右空格
 * @return 返回分割后的字符串
 */
std::vector<std::string> str_split(const std::string &msg, std::string sep, bool trimBlank = false);

/**
 * 截取字符串，直到分隔符
 * @param msg 原始字符串
 * @param seps 分隔符（支持多个）
 * @param keepSep 是否保留分割符
 * @return 截取的字符
 */
std::string str_cut_until_char(std::string &msg, std::string seps, bool keepSep = false);

/**
 * 判断字符串开头字符
 * @param str 原始字符串
 * @param start 要比较的起始字符串
 */
bool str_starts_with(const std::string &str, std::string start);

/**
 * 判断字符串开头字符, 如果为真，裁剪掉开始字符
 * @param str 原始字符串
 * @param start 要比较的起始字符串
 * @param skipSpace 跳过空格
 */
bool str_start_and_cut(std::string &msg, std::string start, bool skipSpace = true);
#endif