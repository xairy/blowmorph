// Copyright (c) 2013 Blowmorph Team

#ifndef BASE_PSTDINT_H_
#define BASE_PSTDINT_H_

#define __STDC_LIMIT_MACROS
#include <stdint.h>

#include "base/macros.h"

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

// Unicode character.
typedef uint32_t uchar_t;

#endif  // BASE_PSTDINT_H_
