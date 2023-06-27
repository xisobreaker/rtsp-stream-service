#include "timeutils.h"

#include <chrono>

using namespace std;
using namespace chrono;

int64_t get_current_timestamp()
{
    system_clock::time_point t = high_resolution_clock::now();
    return duration_cast<microseconds>(t.time_since_epoch()).count();
}