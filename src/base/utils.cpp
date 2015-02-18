// Copyright (c) 2015 Blowmorph Team

#include "base/utils.h"

#include <cstdio>

#include <sstream>
#include <string>

#include "base/macros.h"

namespace bm {

// XXX(xairy): not thread safe because of rand().
size_t Random(size_t max) {
  CHECK(max > 0);
  double zero_to_one = static_cast<double>(rand()) /  // NOLINT
    (static_cast<double>(RAND_MAX) + 1.0f);
  return static_cast<size_t>(zero_to_one * max);
}

std::string IntToStr(int value) {
  std::stringstream ss;
  ss << value;
  return ss.str();
}

}  // namespace bm
