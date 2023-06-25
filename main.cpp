#include <iostream>
using namespace std;

#include "rtsp-client.h"

int main(int argc, char *argv[])
{
    const char rtspUrl[] = "rtsp://127.0.0.1:554/stream";
    RTSPClient client;
    client.connect(rtspUrl);
    return 0;
}
