#include "id_manager.hpp"

#include <base/pstdint.hpp>

namespace bm {

IdManager::IdManager() : _max_id(0) { }
IdManager::~IdManager() { }

uint32_t IdManager::NewId() {
  _max_id += 1;
  return _max_id;
}

} // namespace bm
