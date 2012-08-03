#include "id_manager.hpp"

#include <base/pstdint.hpp>

namespace bm {

uint32_t IdManager::NewId() {
  _max_id += 1;
  return _max_id;
}

IdManager::IdManager() : _max_id(0) { }
IdManager::~IdManager() { }

} // namespace bm
