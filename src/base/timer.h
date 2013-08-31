#ifndef BLOWMORPH_BASE_TIMER_H_
#define BLOWMORPH_BASE_TIMER_H_

// WARNING: Timer is NOT thread safe.

#include <cstdio>
#include <cstdlib>

#ifdef WIN32
# include <ctime>
#else
# include <sys/time.h>
#endif

#include <base/base.h>
#include <base/macros.h>
#include <base/pstdint.h>

namespace bm {

class Timer {
public:
  BM_BASE_DECL Timer();

  // Returns elapsed time in ms since timer's creation.
  BM_BASE_DECL int32_t GetTime() const;

private:
#ifdef WIN32
  clock_t _start;
#else
  timeval _start;
#endif
};

} // namespace bm

#endif // BLOWMORPH_BASE_TIMER_H_
