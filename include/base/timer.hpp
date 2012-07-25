#ifndef BLOWMORPH_BASE_TIMER_HPP_
#define BLOWMORPH_BASE_TIMER_HPP_

// WARNING: Timer is NOT thread safe.

// TODO[24.7.2012 alex]: move code into a .cpp file

#include <ctime>

#include "base/pstdint.hpp"

namespace bm {

class Timer {
public:
  Timer() {
    _clock_start = clock();
  }
  ~Timer() { }

  // Returns elapsed time in ms since timer's creation.
  uint32_t GetTime() const {
    uint64_t clock_diff = static_cast<uint64_t>(clock() - _clock_start);
    uint32_t result = static_cast<uint32_t>(clock_diff * 1000 / CLOCKS_PER_SEC);
    return result;
  }

  // Returns elapsed time since the start of the program.
  static uint32_t GetAbsoluteTime() {
    return clock();
  }

  // Sleeps for 'timeout' ms.
  static void Sleep(uint32_t timeout) {
    uint32_t start = GetAbsoluteTime();
    uint32_t end = start + timeout;
    while(GetAbsoluteTime() < end);
  }

private:
  clock_t _clock_start;
};

} // namespace bm

#endif //BLOWMORPH_BASE_TIMER_HPP_
