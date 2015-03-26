// Copyright (c) 2015 Blowmorph Team

#ifndef BASE_UTILS_H_
#define BASE_UTILS_H_

#include <string>

#include "base/dll.h"

namespace bm {

// Returns random number in the range [0, max).
BM_BASE_DECL size_t Random(size_t max);

BM_BASE_DECL std::string IntToStr(int value);

}  // namespace bm

#endif  // BASE_UTILS_H_
