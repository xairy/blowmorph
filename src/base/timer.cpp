// Copyright (c) 2013 Blowmorph Team

#include "base/timer.h"

#include <limits>

namespace bm {

#ifdef WIN32

/*
Timer::Timer() {
  _start = clock();
}

int32_t Timer::GetTime() const {
  int64_t clock_diff = static_cast<int64_t>(clock() - _start);
  int32_t result = static_cast<int32_t>(clock_diff * 1000 / CLOCKS_PER_SEC);
  return result;
}
*/

Timer::Timer() {
}

// XXX(xairy): this code was not tested.
int32_t Timer::GetTime() const {
    static LARGE_INTEGER nFreq = {0}, nCount = {0};
    static double dReturn = 0.0, dCheckTime = 5.0;

    // Check for a new frequency once every 5 seconds.
    // Note: this is in case ACPI, etc. alters it.
    if (nFreq.QuadPart == 0 || dCheckTime < dReturn) {
      dCheckTime = dReturn + 5.0;

      // Avoid a division by zero by returning zero on error.
      if (!QueryPerformanceFrequency(&nFreq)) {
        struct timeb tp;
        ftime(&tp);
        return static_cast<double>(tp.time) +
               0.001 * static_cast<double>(tp.millitm);
      }
    }

    // Use the ratio of tick amount divided by frequency to find the hertz.
    QueryPerformanceCounter(&nCount);
    dReturn = static_cast<double>nCount.QuadPart /
              static_cast<double>nFreq.QuadPart;

    return static_cast<int32_t>(dReturn * 1000);
}

#else

Timer::Timer() {
  int rv = gettimeofday(&_start, NULL);
  CHECK(rv == 0);
}

int32_t Timer::GetTime() const {
  timeval current;
  int rv = gettimeofday(&current, NULL);
  CHECK(rv == 0);
  int64_t seconds = current.tv_sec - _start.tv_sec;
  int64_t useconds = current.tv_usec - _start.tv_usec;
  int64_t time = seconds * 1000 + useconds / 1000;
  CHECK(0 <= time && time <= std::numeric_limits<int32_t>::max());
  return static_cast<int32_t>(time);
}

#endif

}  // namespace bm
