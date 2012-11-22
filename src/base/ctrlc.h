#ifndef CTRLC_HPP_
#define CTRLC_HPP_

#include "base.hpp"

#ifdef __cplusplus
extern "C" {
#endif

// FIXME[23.11.2012 alex]: Not thread safe.
BM_BASE_DECL void setctrlchandler(void (*handler)());

#ifdef __cplusplus
};
#endif

#endif//CTRLC_HPP_