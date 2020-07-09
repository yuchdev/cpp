#pragma once
#include <chrono>

// Measure elapsed time
class MeasureTime
{
public:
    MeasureTime()
    {
        start = std::chrono::steady_clock::now();
    }

    long long elapsed_msec()
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();
    }

    long long elapsed_mcsec()
    {
        return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - start).count();
    }

    long long elapsed_nsec()
    {
        return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - start).count();
    }

private:
    std::chrono::time_point<std::chrono::steady_clock> start;
};
