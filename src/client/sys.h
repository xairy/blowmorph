// Copyright (c) 2013 SRC Team

#ifndef SRC_CLIENT_SYS_H_
#define SRC_CLIENT_SYS_H_

#include <stdint.h>

namespace sys {

// Returns current timestamp in ms.
int64_t Timestamp();

void Warning(const char* fmt, ...);

}  // namespace sys

#endif  // SRC_CLIENT_SYS_H_
