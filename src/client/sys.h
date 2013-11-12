// Copyright (c) 2013 Blowmorph Team

#ifndef CLIENT_SYS_H_
#define CLIENT_SYS_H_

#include <stdint.h>

namespace sys {

// Returns current timestamp in ms.
int64_t Timestamp();

void Warning(const char* fmt, ...);

}  // namespace sys

#endif  // CLIENT_SYS_H_
