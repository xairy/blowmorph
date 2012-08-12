#ifndef BLOWMORPH_BASE_TIMER_HPP_
#define BLOWMORPH_BASE_TIMER_HPP_

// WARNING: Timer is NOT thread safe.

#include <cstdio>
#include <cstdlib>

#ifdef WIN32
# include <ctime>
#else
# include <sys/time.h>
#endif

#include <base/base.hpp>
#include <base/macros.hpp>
#include <base/pstdint.hpp>

namespace bm {

#ifdef WIN32

class Timer {
public:
  BM_BASE_DECL Timer();

  // Returns elapsed time in ms since timer's creation.
  BM_BASE_DECL uint32_t GetTime() const;

private:
  clock_t _start;
};

#else

class Timer {
public:
  BM_BASE_DECL Timer();

  // Returns elapsed time in ms since timer's creation.
  BM_BASE_DECL uint32_t GetTime() const;

private:
  timeval _start;
};

#endif

} // namespace bm

#endif //BLOWMORPH_BASE_TIMER_HPP_

