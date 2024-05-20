#include "temp.h"

#ifndef MAX_RTSP_SIZE
    #define MAX_RTSP_SIZE 4096
#endif

int rtsp_read_reply(int fd, RTSPMessage *reply, unsigned char **content_ptr, const char *method)
{
    char line_buf[MAX_RTSP_SIZE] = {0};
    int  line_len = 0;

    for (;;) {
        char  ch = '\0';
        char *ptr = line_buf;
        for (int i = 0; i < MAX_RTSP_SIZE; i++) {
            int ret = ::recv(fd, &ch, 1, 0);
            if (ret <= 0) {
                return ret;
            }

            if (ch == '\n') {
                break;
            } else if (ch != '\r') {
                *ptr++ = ch;
            }
        }
        *ptr = '\0';

        if (line_buf[0] == '\0') {
            break;
        }

        if (line_len == 0) {
        } else {
        }
        line_len++;
    }
    return 0;
}