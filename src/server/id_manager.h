// Copyright (c) 2013 Blowmorph Team

#ifndef SERVER_ID_MANAGER_H_
#define SERVER_ID_MANAGER_H_

#include <limits.h>

#include <base/pstdint.h>

namespace bm {

class IdManager {
 public:
  // TODO(xairy): think of something better (signed ids?).
  static const uint32_t BAD_ID = UINT_MAX;

 public:
  IdManager();
  ~IdManager();

  uint32_t NewId();

 private:
  uint32_t _max_id;
};

}  // namespace bm

#endif  // SERVER_ID_MANAGER_H_
