#include "server.hpp"

int run() {
  bm::Server server;
  if(!server.Execute()) {
    bm::Error::Print();
    while(true);
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

int main(int argc, char** argv) {
  int result = run();
  bm::leak_detector::PrintAllLeaks();
  return result;
}
