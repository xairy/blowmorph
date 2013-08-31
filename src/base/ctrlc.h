#ifndef CTRLC_H_
#define CTRLC_H_

#include "base.h"

// FIXME[23.11.2012 alex]: Not thread safe.
BM_BASE_DECL void SetCtrlCHandler(void (*handler)());

#endif // CTRLC_H_
