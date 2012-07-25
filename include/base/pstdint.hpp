#ifndef BLOWMORPH_BASE_PSTDINT_HPP_
#define BLOWMORPH_BASE_PSTDINT_HPP_

#include "base/macros.hpp"

namespace bm {

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

typedef signed char int8_t;
typedef signed short int16_t;
typedef signed int int32_t;
typedef signed long long int64_t;

SCHECK(sizeof(uint8_t) == 1);
SCHECK(sizeof(uint16_t) == 2);
SCHECK(sizeof(uint32_t) == 4);
SCHECK(sizeof(uint64_t) == 8);

SCHECK(sizeof(int8_t) == 1);
SCHECK(sizeof(int16_t) == 2);
SCHECK(sizeof(int32_t) == 4);
SCHECK(sizeof(int64_t) == 8);

typedef float float32;
typedef double float64;
SCHECK(sizeof(float32) == 4);
SCHECK(sizeof(float64) == 8);

// unicode character
typedef uint32_t uchar;

} //namespace bm

#endif //BLOWMORPH_BASE_PSTDINT_HPP_
