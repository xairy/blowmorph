// Copyright (c) 2013 Blowmorph Team

#include "server/id_manager.h"

#include "base/pstdint.h"

namespace bm {

IdManager::IdManager() : _max_id(0) { }
IdManager::~IdManager() { }

uint32_t IdManager::NewId() {
  _max_id += 1;
  return _max_id;
}

}  // namespace bm
