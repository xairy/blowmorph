#include "base/timer.hpp"

namespace bm {

// XXX[alex]: we better measure time as int64_t or double

#ifdef WIN32

Timer::Timer() {
  _start = clock();
}

uint32_t Timer::GetTime() const {
  uint64_t clock_diff = static_cast<uint64_t>(clock() - _start);
  uint32_t result = static_cast<uint32_t>(clock_diff * 1000 / CLOCKS_PER_SEC);
  return result;
}

#else

Timer::Timer() {
  int rv = gettimeofday(&_start, NULL);
  CHECK(rv == 0);
}

uint32_t Timer::GetTime() const {
  timeval current;
  int rv = gettimeofday(&current, NULL);
  long seconds = current.tv_sec - _start.tv_sec;
  long useconds = current.tv_usec - _start.tv_usec;
  long time = seconds * 1000 + useconds / 1000;
  CHECK(time >= 0);
  return static_cast<uint32_t>(time);
}

#endif

}; // namespace bm

