// Copyright (c) 2013 Blowmorph Team

#ifndef SERVER_ID_MANAGER_H_
#define SERVER_ID_MANAGER_H_

#include <base/pstdint.h>

namespace bm {

class IdManager {
 public:
  IdManager();
  ~IdManager();

  uint32_t NewId();

 private:
  uint32_t _max_id;
};

}  // namespace bm

#endif  // SERVER_ID_MANAGER_H_
