// Copyright (c) 2013 Blowmorph Team

#include "client/sys.h"

#include "base/pstdint.h"
#include "base/timer.h"

namespace bm {
namespace sys {

// Warning: not thread safe.
int64_t Timestamp() {
  static bm::Timer timer;
  return timer.GetTime();
}

}  // namespace sys
}  // namespace bm
