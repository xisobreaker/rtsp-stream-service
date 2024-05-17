#include "timeutils.h"

#include <chrono>

using namespace std::chrono;

uint64_t get_current_microseconds()
{
    system_clock::time_point t = high_resolution_clock::now();
    return duration_cast<microseconds>(t.time_since_epoch()).count();
}