#include <stdio.h>

#include <base/ctrlc.hpp>

#include "server.hpp"

// FIXME[23.11.2012 alex]: Not thread safe.
bool global_stop_flag = false;

void CtrlCHandler() {
  global_stop_flag = true;
}

int main(int argc, char** argv) {
  SetCtrlCHandler(&CtrlCHandler);

  bm::Server server;
  
  if (!server.Initialize()) {
    return false;
  }

  printf("Server started.\n");

  while (!global_stop_flag) {
    if (!server.Tick()) {
      bm::Error::Print();
      return EXIT_FAILURE;
    }
  }

  printf("Server finished.\n");

  return EXIT_SUCCESS;
}
