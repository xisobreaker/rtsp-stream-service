#include "rtsp.h"
#include "base64.h"
#include "memutils.h"
#include "rtsputils.h"
#include "strutils.h"
#include "timeutils.h"

#include <chrono>
#include <cmath>
#include <cstring>
#include <iostream>
#include <memory>
#include <string>

using namespace std;
using namespace chrono;

int rtsp_read_reply(int fd, RTSPMessageHeader *reply, unsigned char **content_ptr, const char *method)
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