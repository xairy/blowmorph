// Copyright (c) 2013 Blowmorph Team

#include <cstdlib>

#include "base/error.h"

#include "client/application.h"

int main(int argc, char** argv) {
  bm::Application app;
  if (!app.Initialize()) {
    bm::Error::Print();
    return EXIT_FAILURE;
  }
  if (!app.Run()) {
    bm::Error::Print();
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
