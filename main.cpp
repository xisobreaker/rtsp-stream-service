#include "include/manager/rtsp_client.h"
#include <iostream>
using namespace std;

#include <glog/logging.h>

int main(int argc, char *argv[])
{
    std::string rtspurl = "rtsp://127.0.0.1:554/cam/realmonitor";
    RtspClient  client(rtspurl);
    client.connect();

    return 0;
}
