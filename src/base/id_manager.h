// Copyright (c) 2015 Blowmorph Team

#ifndef BASE_ID_MANAGER_H_
#define BASE_ID_MANAGER_H_

#include <climits>

#include <base/pstdint.h>

#include "base/dll.h"

namespace bm {

class IdManager {
 public:
  static const uint32_t BAD_ID = UINT_MAX;

 public:
  BM_BASE_DECL IdManager();
  BM_BASE_DECL ~IdManager();

  BM_BASE_DECL uint32_t NewId();

 private:
  uint32_t max_id_;
};

}  // namespace bm

#endif  // BASE_ID_MANAGER_H_
