#include "world_manager.hpp"

#include <map>

#include "base/macros.hpp"
#include "base/pstdint.hpp"

#include "entity.hpp"
#include "id_manager.hpp"
#include "vector.hpp"

namespace bm {

WorldManager::WorldManager() { }
WorldManager::~WorldManager() {
  std::map<uint32_t, Entity*>::iterator i;
  for(i = _static_entities.begin(); i != _static_entities.end(); ++i) {
    delete i->second;
  }
  for(i = _dynamic_entities.begin(); i != _dynamic_entities.end(); ++i) {
    delete i->second;
  }
}

void WorldManager::AddEntity(uint32_t id, Entity* entity) {
  CHECK(_static_entities.count(id) == 0 && _dynamic_entities.count(id) == 0);
  if(entity->IsStatic()) {
    _static_entities[id] = entity;
  } else {
    _dynamic_entities[id] = entity;
  }
}

void WorldManager::DeleteEntity(uint32_t id, bool deallocate) {
  CHECK(_static_entities.count(id) + _dynamic_entities.count(id) == 1);
  if(_static_entities.count(id) == 1) {
    if(deallocate) {
      delete _static_entities[id];
    }
    _static_entities.erase(id);
  } else if(_dynamic_entities.count(id) == 1) {
    if(deallocate) {
      delete _dynamic_entities[id];
    }
    _dynamic_entities.erase(id);
  }
}

Entity* WorldManager::GetEntity(uint32_t id) {
  CHECK(_static_entities.count(id) + _dynamic_entities.count(id) == 1);
  if(_static_entities.count(id) == 1) {
    return _static_entities[id];
  } else if(_dynamic_entities.count(id) == 1) {
    return _dynamic_entities[id];
  }
  return NULL;
}

std::map<uint32_t, Entity*>* WorldManager::GetStaticEntities() {
  return &_static_entities;
}

std::map<uint32_t, Entity*>* WorldManager::GetDynamicEntities() {
  return &_dynamic_entities;
}

void WorldManager::GetDestroyedEntities(std::vector<uint32_t>* output) {
  output->clear();

  std::map<uint32_t, Entity*>::iterator itr;
  for(itr = _static_entities.begin(); itr != _static_entities.end(); ++itr) {
    if(itr->second->IsDestroyed()) {
      output->push_back(itr->first);
    }
  }
  for(itr = _dynamic_entities.begin(); itr != _dynamic_entities.end(); ++itr) {
    if(itr->second->IsDestroyed()) {
      output->push_back(itr->first);
    }
  }
}

void WorldManager::DeleteEntities(const std::vector<uint32_t>& input, bool deallocate) {
  size_t size = input.size();
  for(size_t i = 0; i < size; i++) {
    DeleteEntity(input[i], deallocate);
  }
}

void WorldManager::UpdateEntities(uint32_t time) {
  std::map<uint32_t, Entity*>::iterator i;
  for(i = _static_entities.begin(); i != _static_entities.end(); ++i) {
    i->second->Update(time);
  }
  for(i = _dynamic_entities.begin(); i != _dynamic_entities.end(); ++i) {
    i->second->Update(time);
  }
}

void WorldManager::CollideEntities() {
  std::map<uint32_t, Entity*>::iterator i, k;
  for(i = _dynamic_entities.begin(); i != _dynamic_entities.end(); ++i) {
    k = i;
    ++k;
    for(; k != _dynamic_entities.end(); ++k) {
      i->second->Collide(k->second);
    }
  }
  for(i = _dynamic_entities.begin(); i != _dynamic_entities.end(); ++i) {
    for(k = _static_entities.begin(); k != _static_entities.end(); ++k) {
      i->second->Collide(k->second);
    }
  }
}

void WorldManager::DestroyOutlyingEntities(float max_coordinate) {
  // TODO: 'Player' entities should not be destroyed.

  std::map<uint32_t, Entity*>::iterator i;
  for(i = _static_entities.begin(); i != _static_entities.end(); ++i) {
    Entity* entity = i->second;
    Vector2 position = entity->GetPosition();
    if(position.x > max_coordinate || position.y > max_coordinate) {
      entity->Destroy();
    }
  }
  for(i = _dynamic_entities.begin(); i != _dynamic_entities.end(); ++i) {
    Entity* entity = i->second;
    Vector2 position = entity->GetPosition();
    if(position.x > max_coordinate || position.y > max_coordinate) {
      // Temporary solution.
      if(entity->GetSnapshot(1).type != BM_ENTITY_PLAYER) {
        entity->Destroy();
      }
    }
  }
}

bool WorldManager::CreateBullet(
  uint32_t owner_id,
  const Vector2& start,
  const Vector2& end,
  float speed,
  float radius,
  float explosion_radius,
  uint32_t time
) {
  CHECK(_static_entities.count(owner_id) +
    _dynamic_entities.count(owner_id) == 1);
  uint32_t id = Singleton<IdManager>::GetInstance()->NewId();
  Bullet* bullet = Bullet::Create(this, id, owner_id, start, end,
    speed, radius, explosion_radius, time);
  if(bullet == NULL) {
    return false;
  }
  AddEntity(id, bullet);
  //printf("Bullet %u created.\n", id);
  return true;
}

bool WorldManager::CreateDummy(float x, float y, float r, float speed, float radius) {
  uint32_t id = Singleton<IdManager>::GetInstance()->NewId();
  Vector2 position(x, y);
  Dummy* dummy = Dummy::Create(this, id, radius, speed, position, r);
  if(dummy == NULL) {
    return false;
  }
  AddEntity(id, dummy);
  return true;
}

bool WorldManager::CreateWall(float x, float y, float size) {
  uint32_t id = Singleton<IdManager>::GetInstance()->NewId();
  Vector2 position(x, y);
  Wall* wall = Wall::Create(this, id, position, size);
  if(wall == NULL) {
    return false;
  }
  AddEntity(id, wall);
  return true;
}

} // namespace bm
