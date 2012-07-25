#ifndef BLOWMORPH_SERVER_ID_MANAGER_HPP_
#define BLOWMORPH_SERVER_ID_MANAGER_HPP_

#include "base/pstdint.hpp"
#include "base/singleton.hpp"

namespace bm {

class IdManager {
public:
  uint32_t NewId() {
    _max_id += 1;
    return _max_id;
  }

private:
  friend class Singleton<IdManager>;

  IdManager() : _max_id(0) { }
  ~IdManager() { }

  uint32_t _max_id;
};

} //namespace bm

#endif //BLOWMORPH_SERVER_ID_MANAGER_HPP_
