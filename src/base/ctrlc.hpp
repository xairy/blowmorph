#ifndef CTRLC_HPP_
#define CTRLC_HPP_

#include "base.hpp"

// FIXME[23.11.2012 alex]: Not thread safe.
BM_BASE_DECL void SetCtrlCHandler(void (*handler)());

#endif // CTRLC_HPP_
