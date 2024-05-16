#include <iostream>
using namespace std;

#include "rtsp-client.h"

int main(int argc, char *argv[])
{
    const char rtspUrl[] = "rtsp://10.100.1.51:554/cam/realmonitor";
    RTSPClient client;
    client.connect(rtspUrl);
    return 0;
}
