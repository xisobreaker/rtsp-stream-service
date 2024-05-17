#include <iostream>
using namespace std;

#include <glog/logging.h>

int main(int argc, char *argv[])
{
    LOG(INFO) << "hello, world!";
    return 0;
}
