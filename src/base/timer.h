// Copyright (c) 2013 Blowmorph Team

#ifndef SRC_BASE_TIMER_H_
#define SRC_BASE_TIMER_H_

// WARNING: Timer is NOT thread safe.

#ifdef WIN32
# include <time.h>
#else
# include <sys/time.h>
#endif

#include <cstdio>
#include <cstdlib>

#include "base/base.h"
#include "base/macros.h"
#include "base/pstdint.h"

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

}  // namespace bm

#endif  // SRC_BASE_TIMER_H_
