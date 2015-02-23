// Copyright (c) 2015 Blowmorph Team

#include "engine/world.h"

#include <map>

#include <Box2D/Box2D.h>

#include "base/id_manager.h"
#include "base/pstdint.h"

namespace bm {

World::World() : world_(b2Vec2(0.0f, 0.0f)) { }

World::~World() {
  for (auto i : static_entities_) {
    delete i.second;
  }
  for (auto i : dynamic_entities_) {
    delete i.second;
  }
}

b2World* World::GetBox2DWorld() {
  return &world_;
}

Entity* World::GetEntity(uint32_t id) {
  if (static_entities_.count(id) == 1) {
    return static_entities_[id];
  } else if (dynamic_entities_.count(id) == 1) {
    return dynamic_entities_[id];
  }
  return NULL;
}

std::map<uint32_t, Entity*>* World::GetStaticEntities() {
  return &static_entities_;
}

std::map<uint32_t, Entity*>* World::GetDynamicEntities() {
  return &dynamic_entities_;
}

void World::AddEntity(uint32_t id, Entity* entity) {
  CHECK(static_entities_.count(id) == 0 && dynamic_entities_.count(id) == 0);
  if (entity->IsStatic()) {
    static_entities_[id] = entity;
  } else {
    dynamic_entities_[id] = entity;
  }
}

void World::RemoveEntity(uint32_t id) {
  CHECK(static_entities_.count(id) + dynamic_entities_.count(id) == 1);
  Entity* entity = NULL;
  if (static_entities_.count(id) == 1) {
    entity = static_entities_[id];
    static_entities_.erase(id);
  } else if (dynamic_entities_.count(id) == 1) {
    entity = dynamic_entities_[id];
    dynamic_entities_.erase(id);
  }
  CHECK(entity != NULL);
}

}  // namespace bm
