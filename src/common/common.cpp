/** @file       common.cpp
 *  @brief
 *  @author     Mei Zhaorui
 *  @version    1.0
 *  @date       2023-07-26
 *  @copyright  Heli Co., Ltd. All rights reserved.
 */
#include "common.h"

#ifdef WIN32
#include <Windows.h>

namespace
{
#define MS_PER_SEC 1000ULL  // MS = milliseconds
#define US_PER_MS  1000ULL  // US = microseconds
#define HNS_PER_US 10ULL    // HNS = hundred-nanoseconds (e.g., 1 hns = 100 ns)
#define NS_PER_US  1000ULL

#define HNS_PER_SEC (MS_PER_SEC * US_PER_MS * HNS_PER_US)
#define NS_PER_HNS  (100ULL)  // NS = nanoseconds
#define NS_PER_SEC  (MS_PER_SEC * US_PER_MS * NS_PER_US)

struct timespec
{
    long tv_sec;
    long tv_nsec;
};

enum clockid_t
{
    CLOCK_MONOTONIC,
    CLOCK_REALTIME
};

int clock_gettime_monotonic(struct timespec* tv)
{
    static LARGE_INTEGER ticksPerSec;
    LARGE_INTEGER ticks;

    if (!ticksPerSec.QuadPart)
    {
        QueryPerformanceFrequency(&ticksPerSec);
        if (!ticksPerSec.QuadPart)
        {
            errno = ENOTSUP;
            return -1;
        }
    }

    QueryPerformanceCounter(&ticks);

    tv->tv_sec = (long)(ticks.QuadPart / ticksPerSec.QuadPart);
    tv->tv_nsec = (long)(((ticks.QuadPart % ticksPerSec.QuadPart) * NS_PER_SEC) / ticksPerSec.QuadPart);

    return 0;
}

int clock_gettime_realtime(struct timespec* tv)
{
    FILETIME ft;
    ULARGE_INTEGER hnsTime;

    GetSystemTimePreciseAsFileTime(&ft);

    hnsTime.LowPart = ft.dwLowDateTime;
    hnsTime.HighPart = ft.dwHighDateTime;

    // To get POSIX Epoch as baseline, subtract the number of hns intervals from Jan 1, 1601 to Jan 1, 1970.
    hnsTime.QuadPart -= (11644473600ULL * HNS_PER_SEC);

    // modulus by hns intervals per second first, then convert to ns, as not to lose resolution
    tv->tv_nsec = (long)((hnsTime.QuadPart % HNS_PER_SEC) * NS_PER_HNS);
    tv->tv_sec = (long)(hnsTime.QuadPart / HNS_PER_SEC);

    return 0;
}

int clock_gettime(clockid_t type, struct timespec* tp)
{
    switch (type)
    {
    case CLOCK_MONOTONIC:
        return clock_gettime_monotonic(tp);
    case CLOCK_REALTIME:
        return clock_gettime_realtime(tp);
    default:
        return -1;
    }
}

}  // namespace
#else
#include <time.h>
#endif

namespace lingxi::vtk
{

int64_t GetCurrentTimestamp()
{
    struct timespec ts = {0, 0};
    clock_gettime(CLOCK_REALTIME, &ts);

    return (int64_t)ts.tv_sec * (int64_t)1e9 + (int64_t)ts.tv_nsec;
}

}  // namespace lingxi::vtk
