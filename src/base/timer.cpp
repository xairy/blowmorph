// Copyright (c) 2013 Blowmorph Team

#include "base/timer.h"

#include <limits>

namespace bm {

#ifdef WIN32

Timer::Timer() {
  _start = clock();
}

int64_t Timer::GetTime() const {
  int64_t clock_diff = static_cast<int64_t>(clock() - _start);
  int64_t result = clock_diff * 1000 / CLOCKS_PER_SEC;
  return result;
}

#else

Timer::Timer() {
  int rv = gettimeofday(&_start, NULL);
  CHECK(rv == 0);
}

int64_t Timer::GetTime() const {
  timeval current;
  int rv = gettimeofday(&current, NULL);
  CHECK(rv == 0);
  int64_t seconds = current.tv_sec - _start.tv_sec;
  int64_t useconds = current.tv_usec - _start.tv_usec;
  int64_t time = seconds * 1000 + useconds / 1000;
  return time;
}

#endif

}  // namespace bm
