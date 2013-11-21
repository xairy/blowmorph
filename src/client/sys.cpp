// Copyright (c) 2013 Blowmorph Team

#include "client/sys.h"

#if defined(WIN32)
  #include <windows.h>
#else
  #include <sys/time.h>
#endif

#include <cstdarg>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cassert>

#include "base/timer.h"

namespace sys {

int64_t Timestamp() {
  static bm::Timer timer;
  return timer.GetTime();
}

}  // namespace sys
