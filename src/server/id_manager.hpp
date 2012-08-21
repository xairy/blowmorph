#ifndef BLOWMORPH_SERVER_ID_MANAGER_HPP_
#define BLOWMORPH_SERVER_ID_MANAGER_HPP_

#include <base/pstdint.hpp>

namespace bm {

class IdManager {
public:
  IdManager();
  ~IdManager();

  uint32_t NewId();

private:
  uint32_t _max_id;
};

} //namespace bm

#endif //BLOWMORPH_SERVER_ID_MANAGER_HPP_
