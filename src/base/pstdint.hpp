#ifndef BLOWMORPH_BASE_PSTDINT_HPP_
#define BLOWMORPH_BASE_PSTDINT_HPP_

#include "base/macros.hpp"

#include <stdint.h>

typedef float float32_t;
typedef double float64_t;

SCHECK(sizeof(uint8_t) == 1);
SCHECK(sizeof(uint16_t) == 2);
SCHECK(sizeof(uint32_t) == 4);
SCHECK(sizeof(uint64_t) == 8);

SCHECK(sizeof(int8_t) == 1);
SCHECK(sizeof(int16_t) == 2);
SCHECK(sizeof(int32_t) == 4);
SCHECK(sizeof(int64_t) == 8);

SCHECK(sizeof(float32_t) == 4);
SCHECK(sizeof(float64_t) == 8);

// unicode character
typedef uint32_t uchar_t;

#endif //BLOWMORPH_BASE_PSTDINT_HPP_
