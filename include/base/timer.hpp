#ifndef BLOWMORPH_BASE_TIMER_HPP_
#define BLOWMORPH_BASE_TIMER_HPP_

// WARNING: Timer is NOT thread safe.

#include <ctime>

#include "base/pstdint.hpp"

namespace bm {

class Timer {
public:
  Timer();
  ~Timer();

  // Returns elapsed time in ms since timer's creation.
  uint32_t GetTime() const;

  // Returns elapsed time since the start of the program.
  static uint32_t GetAbsoluteTime();

  // Sleeps for 'timeout' ms.
  static void Sleep(uint32_t timeout);

private:
  clock_t _clock_start;
};

} // namespace bm

#endif //BLOWMORPH_BASE_TIMER_HPP_
