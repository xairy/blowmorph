// Copyright (c) 2013 Blowmorph Team

#ifndef BASE_TIME_H_
#define BASE_TIME_H_

#include "base/pstdint.h"

namespace bm {

// Returns time since some moment in ms.
// Warning: not thread safe.
int64_t Timestamp();

}  // namespace bm

#endif  // BASE_TIME_H_
