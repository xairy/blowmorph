#include <base/timer.hpp>

namespace bm {

// TODO[alex]: code doesn't work on linux, fix it
// XXX[alex]: we better measure time as int64_t or double

Timer::Timer() {
  _clock_start = clock();
}
Timer::~Timer() { }

// Returns elapsed time in ms since timer's creation.
uint32_t Timer::GetTime() const {
  uint64_t clock_diff = static_cast<uint64_t>(clock() - _clock_start);
  uint32_t result = static_cast<uint32_t>(clock_diff * 1000 / CLOCKS_PER_SEC);
  return result;
}

// Returns elapsed time since the start of the program.
uint32_t Timer::GetAbsoluteTime() {
  return clock();
}

// Sleeps for 'timeout' ms.
void Timer::Sleep(uint32_t timeout) {
  uint32_t start = GetAbsoluteTime();
  uint32_t end = start + timeout;
  while(GetAbsoluteTime() < end);
}

}; // namespace bm
