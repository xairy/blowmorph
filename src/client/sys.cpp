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
  return static_cast<int64_t>(timer.GetTime());
}

void Warning(const char* fmt, ...) {
  assert(fmt != NULL);

  char buf[1024];

  va_list args;
  va_start(args, fmt);
  vsnprintf(buf, sizeof(buf) - 1, fmt, args);
  va_end(args);

  fprintf(stderr, "WARN: %s\n", buf);
}

}  // namespace sys
