#include "server.hpp"

#include <base/ctrlc.h>

// FIXME[23.11.2012 alex]: Not thread safe.
volatile bool global_stop_flag = false;

void CtrlCHandler() {
  global_stop_flag = true;
}

int run() {
  setctrlchandler(CtrlCHandler);

  bm::Server server;
  
  if(!server.Initialize()) {
    return false;
  }

  printf("Server started.\n");

  while(!global_stop_flag) {
    if(!server.Tick()) {
      bm::Error::Print();
      return EXIT_FAILURE;
    }
  }

  printf("Server finished.\n");

  return EXIT_SUCCESS;
}

int main(int argc, char** argv) {
  int result = run();
  //bm::leak_detector::PrintAllLeaks();
  return result;
}
