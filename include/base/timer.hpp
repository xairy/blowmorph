#ifndef BLOWMORPH_BASE_TIMER_HPP_
#define BLOWMORPH_BASE_TIMER_HPP_

// WARNING: Timer is NOT thread safe.

#ifdef WIN32
# include <ctime>
#else
# include "sys/time.h"
#endif

#include "base/macros.hpp"
#include "base/pstdint.hpp"

namespace bm {

#ifdef WIN32

class Timer {
public:
  Timer() {
    _start = clock();
  }
  ~Timer() { }

  // Returns elapsed time in ms since timer's creation.
  uint32_t GetTime() const {
    uint64_t clock_diff = static_cast<uint64_t>(clock() - _start);
    uint32_t result = static_cast<uint32_t>(clock_diff * 1000 / CLOCKS_PER_SEC);
    return result;
  }

private:
  clock_t _start;
};

#else

class Timer {
public:
  Timer() {
    int rv = gettimeofday(&_start, NULL);
    CHECK(rv == 0);
  }

  uint32_t GetTime() const {
    timeval current;
    int rv = gettimeofday(&current, NULL);
    long seconds = current.tv_sec - _start.tv_sec;
    long useconds = current.tv_usec - _start.tv_usec;
    long time = seconds * 1000 + useconds / 1000;
    CHECK(time >= 0);
    return static_cast<uint32_t>(time);
  }

private:
  timeval _start;
};

#endif

} // namespace bm

#endif //BLOWMORPH_BASE_TIMER_HPP_

