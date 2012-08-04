#include "world_manager.hpp"

#include <map>

#include <pugixml.hpp>

#include <base/error.hpp>
#include <base/macros.hpp>
#include <base/pstdint.hpp>

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
  
  std::map<uint32_t, Entity*>::iterator itr;
  for(itr = _static_entities.begin(); itr != _static_entities.end(); ++itr) {
    itr->second->OnEntityAppearance(entity);
    entity->OnEntityAppearance(itr->second);
  }
  for(itr = _dynamic_entities.begin(); itr != _dynamic_entities.end(); ++itr) {
    itr->second->OnEntityAppearance(entity);
    entity->OnEntityAppearance(itr->second);
  }
}

void WorldManager::DeleteEntity(uint32_t id, bool deallocate) {
  CHECK(_static_entities.count(id) + _dynamic_entities.count(id) == 1);
  Entity* entity = NULL;
  if(_static_entities.count(id) == 1) {
    entity = _static_entities[id];
    _static_entities.erase(id);
  } else if(_dynamic_entities.count(id) == 1) {
    entity = _dynamic_entities[id];
    _dynamic_entities.erase(id);
  }
  CHECK(entity != NULL);

  std::map<uint32_t, Entity*>::iterator itr;
  for(itr = _static_entities.begin(); itr != _static_entities.end(); ++itr) {
    itr->second->OnEntityDisappearance(entity);
  }
  for(itr = _dynamic_entities.begin(); itr != _dynamic_entities.end(); ++itr) {
    itr->second->OnEntityDisappearance(entity);
  }

  if(deallocate) {
    delete entity;
  }
}

void WorldManager::DeleteEntities(const std::vector<uint32_t>& input, bool deallocate) {
  size_t size = input.size();
  for(size_t i = 0; i < size; i++) {
    DeleteEntity(input[i], deallocate);
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
      // Temporary solution?
      if(entity->GetType() != "Player") {
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
  return true;
}

bool WorldManager::CreateDummy(
  const Vector2& position,
  float radius,
  float speed,
  uint32_t time
) {
  uint32_t id = Singleton<IdManager>::GetInstance()->NewId();
  Dummy* dummy = Dummy::Create(this, id, position, radius, speed, time);
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

bool WorldManager::LoadMap(const std::string& file) {
  pugi::xml_document document;
  pugi::xml_parse_result parse_result = document.load_file(file.c_str());
  if(!parse_result) {
    Error::Throw(__FILE__, __LINE__, "Unable to parse %s!\n", file.c_str());
    return false;
  }
  pugi::xml_node map_node = document.child("map");
  if(!map_node) {
    Error::Throw(__FILE__, __LINE__, "Incorrect format of %s!\n", file.c_str());
    return false;
  }

  for(pugi::xml_node node = map_node.first_child(); node; node = node.next_sibling()) {
    if(std::string(node.name()) == "wall") {
      pugi::xml_attribute x = node.attribute("x");
      pugi::xml_attribute y = node.attribute("y");
      pugi::xml_attribute size = node.attribute("size");
      if(!x || !y || !size) {
        fprintf(stderr, "Warning: incorrect xml node format, ignored.\n");
      } else {
        bool rv = CreateWall(x.as_float(), y.as_float(), size.as_float());
        if(rv == false) {
          return false;
        }
      }
    } else if(std::string(node.name()) == "chunk") {
      pugi::xml_attribute x = node.attribute("x");
      pugi::xml_attribute y = node.attribute("y");
      pugi::xml_attribute width = node.attribute("width");
      pugi::xml_attribute height = node.attribute("height");
      pugi::xml_attribute size = node.attribute("size");
      if(!x || !y || !width || !height || !size) {
        fprintf(stderr, "Warning: incorrect xml node format, ignored.\n");
      } else {
        float x_value = x.as_float();
        float y_value = y.as_float();
        int w_value = width.as_int();
        int h_value = height.as_int();
        float s_value = size.as_float();
        for(int i = 0; i < w_value; i++) {
          for(int j = 0; j < h_value; j++) {
            bool rv = CreateWall(x_value + i * s_value, y_value + j * s_value, s_value);
            if(rv == false) {
              return false;
            }
          }
        }
      }
    }
  }

  return true;
}

} // namespace bm
