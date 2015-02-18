// Copyright (c) 2015 Blowmorph Team

#ifndef BASE_UTILS_H_
#define BASE_UTILS_H_

#include <string>

namespace bm {

// Returns random number in the range [0, max).
size_t Random(size_t max);

std::string IntToStr(int value);

}  // namespace bm

#endif  // BASE_UTILS_H_
