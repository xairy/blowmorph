// Copyright (c) 2015 Blowmorph Team

#include "base/id_manager.h"

#include <climits>

#include "base/macros.h"
#include "base/pstdint.h"

namespace bm {

IdManager::IdManager() : max_id_(0) { }
IdManager::~IdManager() { }

uint32_t IdManager::NewId() {
  max_id_ += 1;
  CHECK(max_id_ < UINT_MAX);
  return max_id_;
}

}  // namespace bm
