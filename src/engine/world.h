// Copyright (c) 2015 Blowmorph Team

#ifndef ENGINE_WORLD_H_
#define ENGINE_WORLD_H_

#include <map>

#include <Box2D/Box2D.h>

#include "base/id_manager.h"
#include "base/pstdint.h"

#include "engine/entity.h"

namespace bm {

class World {
 public:
  explicit World();
  virtual ~World();

  b2World* GetBox2DWorld();

  Entity* GetEntity(uint32_t id);
  std::map<uint32_t, Entity*>* GetStaticEntities();
  std::map<uint32_t, Entity*>* GetDynamicEntities();

  // 'RemoveEntity()' doesn't delete the entity object.
  void AddEntity(uint32_t id, Entity* entity);
  void RemoveEntity(uint32_t id);

 private:
  b2World world_;

  std::map<uint32_t, Entity*> static_entities_;
  std::map<uint32_t, Entity*> dynamic_entities_;
};

}  // namespace bm

#endif  // ENGINE_WORLD_H_
