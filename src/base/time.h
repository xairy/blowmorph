// Copyright (c) 2013 Blowmorph Team

#ifndef BASE_TIME_H_
#define BASE_TIME_H_

#include "base/dll.h"
#include "base/pstdint.h"

namespace bm {

// Returns time since some moment in ms.
BM_BASE_DECL int64_t Timestamp();

}  // namespace bm

#endif  // BASE_TIME_H_
