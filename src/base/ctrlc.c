#include "ctrlc.h"

#ifdef _WIN32
  #include <Windows.h>
#else
  #include <signal.h>
#endif

static void (*sighandler)();

#ifdef _WIN32
BOOL WINAPI WinHandler(DWORD dwEvent) {
  if (dwEvent == CTRL_C_EVENT) {
    sighandler();
  }
  return TRUE;
}
#else
void unix_handler(int s) {
  if (s == SIGINT)
    sighandler();
  }
}
#endif

// FIXME[23.11.2012 alex]: Not thread safe.
void setctrlchandler(void (*handler)()) {
  sighandler = handler;
  
  #ifdef _WIN32
    SetConsoleCtrlHandler((PHANDLER_ROUTINE) WinHandler, TRUE);
  #else
    signal(SIGINT, &unix_handler);
  #endif
}