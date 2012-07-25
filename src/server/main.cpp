#include "server.hpp"

int main(int argc, char** argv) {
  bm::Server server;
  if(!server.Execute()) {
    bm::Error::Print();
    while(true);
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
