// Copyright (c) 2015 Blowmorph Team

#ifndef ENGINE_WORLD_H_
#define ENGINE_WORLD_H_

#include <map>

#include <Box2D/Box2D.h>

#include "base/id_manager.h"
#include "base/pstdint.h"

#include "engine/dll.h"
#include "engine/entity.h"

namespace bm {

class World {
 public:
  BM_ENGINE_DECL explicit World();
  BM_ENGINE_DECL virtual ~World();

  BM_ENGINE_DECL b2World* GetBox2DWorld();

  BM_ENGINE_DECL Entity* GetEntity(uint32_t id);
  BM_ENGINE_DECL std::map<uint32_t, Entity*>* GetStaticEntities();
  BM_ENGINE_DECL std::map<uint32_t, Entity*>* GetDynamicEntities();

  // 'RemoveEntity()' doesn't delete the entity object.
  BM_ENGINE_DECL void AddEntity(uint32_t id, Entity* entity);
  BM_ENGINE_DECL void RemoveEntity(uint32_t id);

 private:
  b2World world_;

  std::map<uint32_t, Entity*> static_entities_;
  std::map<uint32_t, Entity*> dynamic_entities_;
};

}  // namespace bm

#endif  // ENGINE_WORLD_H_
