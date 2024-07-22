#include "rtsp_url.h"
#include "rtsp_def.h"
#include "strutils.h"
#include <cstdlib>

void split_video_url(RTSPUrlInfo *info, const char *url)
{
    const char *pos = nullptr;
    if ((pos = strchr(url, ':'))) {
        str_copy(info->proto, url, (pos + 1 - url));

        pos++; // skip ':'
        if (*pos == '/')
            pos++;
        if (*pos == '/')
            pos++;
    } else {
        str_copy(info->path, url, sizeof(info->path));
        return;
    }

    const char *lstr = pos + strcspn(pos, "/?#");
    str_copy(info->path, lstr, sizeof(info->path));

    if (lstr != pos) {
        const char *begin = pos, *end = nullptr;
        while ((end = strchr(pos, '@')) && end < lstr) {
            str_copy(info->authorization, begin, end + 1 - begin);
            pos = end + 1;
        }

        const char *brk = nullptr, *col = nullptr;
        if (*pos == '[' && (brk = strchr(pos, ']')) && brk < lstr) {
            str_copy(info->hostname, pos + 1, brk - pos);
            if (brk[1] == ':') {
                info->port = atoi(brk + 2);
            }
        } else if ((col = strchr(pos, ':')) && col < lstr) {
            str_copy(info->hostname, pos, col + 1 - pos);
            info->port = atoi(col + 1);
        } else {
            str_copy(info->hostname, pos, lstr + 1 - pos);
            info->port = RTSP_DEFAULT_PORT;
        }
    }
}