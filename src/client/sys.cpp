#include "sys.hpp"

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

namespace sys {

double Timestamp() {
  #if defined (WIN32)
    static LARGE_INTEGER nFreq = {0}, nCount = {0};
    static double dReturn = 0.0, dCheckTime = 5.0;

    // check for a new frequency once every 5 seconds
    // note: this is in case ACPI, etc. alters it
    if((nFreq.QuadPart == 0) || (dCheckTime < dReturn)) {
      dCheckTime = dReturn + 5.0;

      // avoid a division by zero by returning zero on error
      if (!QueryPerformanceFrequency(&nFreq)) {
        struct timeb tp;
        ftime(&tp);
        return (double) tp.time + 0.001 * (double) tp.millitm;
      }
    }

    // use the ratio of tick amount divided by frequency to find the hertz
    QueryPerformanceCounter(&nCount);
    dReturn = ((double)nCount.QuadPart / (double)nFreq.QuadPart);
    return dReturn;
  #else
    static struct timeval time;
    gettimeofday(&time, 0);
    static long secsFirstCall = time.tv_sec;
    return (double) (time.tv_sec-secsFirstCall) +
      (double) time.tv_usec / (1000.0 * 1000.0);
  #endif
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