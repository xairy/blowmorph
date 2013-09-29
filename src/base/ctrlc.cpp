// Copyright (c) 2013 Blowmorph Team

#include "base/ctrlc.h"

#ifdef _WIN32
  #include <Windows.h>
#else
  #include <signal.h>
#endif

#include "base/macros.h"

static void (*signal_handler)();

#ifdef _WIN32
BOOL WINAPI WinHandler(DWORD dwEvent) {
  if (dwEvent == CTRL_C_EVENT) {
    signal_handler();
  }
  return TRUE;
}
#else
void UnixHandler(int s) {
  if (s == SIGINT) {
    signal_handler();
  }
}
#endif

// FIXME[23.11.2012 alex]: Not thread safe.
void SetCtrlCHandler(void (*handler)()) {
  signal_handler = handler;

  #ifdef _WIN32
    SetConsoleCtrlHandler((PHANDLER_ROUTINE)WinHandler, TRUE);
    // FIXME[xairy]: check return value.
  #else
    sighandler_t rv = signal(SIGINT, &UnixHandler);
    CHECK(rv != SIG_ERR);
  #endif
}
