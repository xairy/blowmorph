#ifndef BLOWMORPH_CLIENT_SYS_HPP_
#define BLOWMORPH_CLIENT_SYS_HPP_

#include <stdint.h>

namespace sys {

// Returns current timestamp in ms.
int64_t Timestamp();

void Warning(const char* fmt, ...);

} // namespace sys

#endif // BLOWMORPH_CLIENT_SYS_HPP_