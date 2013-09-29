// Copyright (c) 2013 Blowmorph Team

#ifndef SRC_BASE_CTRLC_H_
#define SRC_BASE_CTRLC_H_

#include <base/base.h>

// FIXME[23.11.2012 alex]: Not thread safe.
BM_BASE_DECL void SetCtrlCHandler(void (*handler)());

#endif  // SRC_BASE_CTRLC_H_
