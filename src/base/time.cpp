// Copyright (c) 2013 Blowmorph Team

#include "base/time.h"

#include "base/pstdint.h"
#include "base/timer.h"

namespace bm {

int64_t Timestamp() {
  static bm::Timer timer;
  return timer.GetTime();
}

}  // namespace bm
