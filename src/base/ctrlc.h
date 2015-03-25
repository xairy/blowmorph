// Copyright (c) 2013 Blowmorph Team

#ifndef BASE_CTRLC_H_
#define BASE_CTRLC_H_

#include "base/dll.h"

// FIXME(alex): Not thread safe.
BM_BASE_DECL void SetCtrlCHandler(void (*handler)());

#endif  // BASE_CTRLC_H_
