#include "world_manager.hpp"

#include <cmath>
#include <cstdlib>

#include <map>

#include <pugixml.hpp>

#include <base/error.hpp>
#include <base/macros.hpp>
#include <base/pstdint.hpp>

#include "entity.hpp"
#include "id_manager.hpp"
#include "vector.hpp"
#include "shape.hpp"

#include "bullet.hpp"
#include "dummy.hpp"
#include "player.hpp"
#include "wall.hpp"

namespace {
  double round(double value) {
    return ::floor(value + 0.5);
  }
  float round(float value) {
    return ::floorf(value + 0.5f);
  }
}

namespace bm {

WorldManager::WorldManager() : _map_type(MAP_NONE) { }
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
    if(abs(position.x) > max_coordinate || abs(position.y) > max_coordinate) {
      entity->Destroy();
    }
  }
  for(i = _dynamic_entities.begin(); i != _dynamic_entities.end(); ++i) {
    Entity* entity = i->second;
    Vector2 position = entity->GetPosition();
    if(abs(position.x) > max_coordinate || abs(position.y) > max_coordinate) {
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

bool WorldManager::CreateWall(const Vector2& position, float size, Wall::Type type) {
  uint32_t id = Singleton<IdManager>::GetInstance()->NewId();
  Wall* wall = Wall::Create(this, id, position, size, type);
  if(wall == NULL) {
    return false;
  }
  AddEntity(id, wall);
  return true;
}

bool WorldManager::CreateAlignedWall(float x, float y, Wall::Type type) {
  CHECK(_map_type == MAP_GRID);

  int xa = static_cast<int>(round(x / _block_size));
  int ya = static_cast<int>(round(y / _block_size));

  return _CreateAlignedWall(xa, ya, type);
}

bool WorldManager::_CreateAlignedWall(int x, int y, Wall::Type type) {
  CHECK(_map_type == MAP_GRID);

  return CreateWall(Vector2(x * _block_size, y * _block_size), _block_size, type);
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
    Error::Throw(__FILE__, __LINE__, "Tag 'map' not found in %s!\n", file.c_str());
    return false;
  }

  pugi::xml_attribute map_type = map_node.attribute("type");
  if(!map_type) {
    Error::Throw(__FILE__, __LINE__, "Tag 'map' does not have attribute 'type' in %s!\n", file.c_str());
    return false;
  }

  if(std::string(map_type.value()) == "grid") {
    pugi::xml_attribute block_size = map_node.attribute("block_size");
    if(!block_size) {
      Error::Throw(__FILE__, __LINE__, "Tag 'map' does not have attribute 'block_size' in %s!\n", file.c_str());
      return false;
    }
    _block_size = block_size.as_float();
    _map_type = MAP_GRID;
  } else {
    _map_type = MAP_ARBITRARY;
  }

  for(pugi::xml_node node = map_node.first_child(); node; node = node.next_sibling()) {
    if(std::string(node.name()) == "wall") {
      if(!_LoadWall(node)) {
        return false;
      }
    } else if(std::string(node.name()) == "chunk") {
      if(!_LoadChunk(node)) {
        return false;
      }
    } else if(std::string(node.name()) == "spawn") {
      if(!_LoadSpawn(node)) {
        return false;
      }
    }
  }

  return true;
}

// TODO: refactor.
bool WorldManager::_LoadWall(const pugi::xml_node& node) {
  CHECK(_map_type != MAP_NONE);
  CHECK(std::string(node.name()) == "wall");

  if(_map_type == MAP_ARBITRARY) {
    pugi::xml_attribute x = node.attribute("x");
    pugi::xml_attribute y = node.attribute("y");
    pugi::xml_attribute size = node.attribute("size");
    pugi::xml_attribute type = node.attribute("type");
    if(!x || !y || !size || !type) {
      Error::Throw(__FILE__, __LINE__, "Incorrect format of 'wall' in map file!\n");
      return false;
    } else {
      Wall::Type type_value;
      bool rv = _LoadWallType(type, &type_value);
      if(rv == false) {
        return false;
      }
      rv = CreateWall(Vector2(x.as_float(), y.as_float()), size.as_float(), type_value);
      if(rv == false) {
        return false;
      }
    }
  } else if(_map_type == MAP_GRID) {
    pugi::xml_attribute x = node.attribute("x");
    pugi::xml_attribute y = node.attribute("y");
    pugi::xml_attribute type = node.attribute("type");
    if(!x || !y || !type) {
      Error::Throw(__FILE__, __LINE__, "Incorrect format of 'wall' in map file!\n");
      return false;
    } else {
      Wall::Type type_value;
      bool rv = _LoadWallType(type, &type_value);
      if(rv == false) {
        return false;
      }
      rv = _CreateAlignedWall(x.as_int(), y.as_int(), type_value);
      if(rv == false) {
        return false;
      }
    }
  }

  return true;
}

// TODO: refactor.
bool WorldManager::_LoadChunk(const pugi::xml_node& node) {
  CHECK(_map_type != MAP_NONE);
  CHECK(std::string(node.name()) == "chunk");

  if(_map_type == MAP_ARBITRARY) {
    pugi::xml_attribute x = node.attribute("x");
    pugi::xml_attribute y = node.attribute("y");
    pugi::xml_attribute width = node.attribute("width");
    pugi::xml_attribute height = node.attribute("height");
    pugi::xml_attribute size = node.attribute("size");
    pugi::xml_attribute type = node.attribute("type");
    if(!x || !y || !width || !height || !size || !type) {
      Error::Throw(__FILE__, __LINE__, "Incorrect format of 'chunk' in map file!\n");
      return false;
    } else {
      float xv = x.as_float();
      float yv = y.as_float();
      int wv = width.as_int();
      int hv = height.as_int();
      float sv = size.as_float();
      Wall::Type type_value;
      bool rv = _LoadWallType(type, &type_value);
      if(rv == false) {
        return false;
      }
      for(int i = 0; i < wv; i++) {
        for(int j = 0; j < hv; j++) {
          bool rv = CreateWall(Vector2(xv + i * sv, yv + j * sv), sv, type_value);
          if(rv == false) {
            return false;
          }
        }
      }
    }
  } else if(_map_type == MAP_GRID) {
    pugi::xml_attribute x = node.attribute("x");
    pugi::xml_attribute y = node.attribute("y");
    pugi::xml_attribute width = node.attribute("width");
    pugi::xml_attribute height = node.attribute("height");
    pugi::xml_attribute type = node.attribute("type");
    if(!x || !y || !width || !height || !type) {
      Error::Throw(__FILE__, __LINE__, "Incorrect format of 'chunk' in map file!\n");
      return false;
    } else {
      int xv = x.as_int();
      int yv = y.as_int();
      int wv = width.as_int();
      int hv = height.as_int();
      Wall::Type type_value;
      bool rv = _LoadWallType(type, &type_value);
      if(rv == false) {
        return false;
      }
      for(int i = 0; i < wv; i++) {
        for(int j = 0; j < hv; j++) {
          bool rv = _CreateAlignedWall(xv + i, yv + j, type_value);
          if(rv == false) {
            return false;
          }
        }
      }
    }
  }

  return true;
}

bool WorldManager::_LoadSpawn(const pugi::xml_node& node) {
  CHECK(_map_type != MAP_NONE);
  CHECK(std::string(node.name()) == "spawn");

  if(_map_type == MAP_ARBITRARY) {
    pugi::xml_attribute x = node.attribute("x");
    pugi::xml_attribute y = node.attribute("y");
    if(!x || !y) {
      Error::Throw(__FILE__, __LINE__, "Incorrect format of 'spawn' in map file!\n");
      return false;
    } else {
      _spawn_positions.push_back(Vector2(x.as_float(), y.as_float()));
    }
  } else if(_map_type == MAP_GRID) {
    pugi::xml_attribute x = node.attribute("x");
    pugi::xml_attribute y = node.attribute("y");
    if(!x || !y) {
      Error::Throw(__FILE__, __LINE__, "Incorrect format of 'spawn' in map file!\n");
      return false;
    } else {
      float xv = x.as_float() * _block_size;
      float yv = y.as_float() * _block_size;
      _spawn_positions.push_back(Vector2(xv, yv));
    }
  }
  return true;
}

bool WorldManager::_LoadWallType(const pugi::xml_attribute& attribute, Wall::Type* output) {
  CHECK(std::string(attribute.name()) == "type");
  if(std::string(attribute.value()) == "ordinary") {
    *output = Wall::TYPE_ORDINARY;
  } else if(std::string(attribute.value()) == "unbreakable") {
    *output = Wall::TYPE_UNBREAKABLE;
  } else if(std::string(attribute.value()) == "morphed") {
    *output = Wall::TYPE_MORPHED;
  } else {
    Error::Throw(__FILE__, __LINE__, "Incorrect wall type in map file!\n");
    return false;
  }
  return true;
}

bool WorldManager::Blow(const Vector2& location, float radius) {
  Shape* explosion = new Circle(location, radius);
  if(explosion == NULL) {
    Error::Set(Error::TYPE_MEMORY);
    return false;
  }

  std::map<uint32_t, Entity*>::iterator i;
  for(i = _static_entities.begin(); i != _static_entities.end(); ++i) {
    Entity* entity = i->second;
    if(explosion->Collide(entity->GetShape())) {
      entity->Damage();
    }
  }
  for(i = _dynamic_entities.begin(); i != _dynamic_entities.end(); ++i) {
    Entity* entity = i->second;
    if(explosion->Collide(entity->GetShape())) {
      entity->Damage();
    }
  }

  return true;
}

bool WorldManager::Morph(const Vector2& location, int radius) {
  int lx = static_cast<int>(round(static_cast<float>(location.x) / _block_size));
  int ly = static_cast<int>(round(static_cast<float>(location.y) / _block_size));
  for(int x = -radius; x <= radius; x++) {
    for(int y = -radius; y <= radius; y++) {
      if(x * x + y * y <= radius * radius) {
        bool rv = _CreateAlignedWall(lx + x, ly + y, Wall::TYPE_MORPHED);
        if(rv == false) {
          return false;
        }
      }
    }
  }
  return true;
}

namespace {

// Returns random number in the range [0, max).
size_t Random(size_t max) {
  CHECK(max > 0);
  double zero_to_one = static_cast<double>(rand()) / (static_cast<double>(RAND_MAX) + 1.0f);
  return static_cast<size_t>(zero_to_one * max);
}

} // anonymous namespace

Vector2 WorldManager::GetRandomSpawn() const {
  CHECK(_spawn_positions.size() > 0);

  size_t spawn_count = _spawn_positions.size();
  size_t spawn = Random(spawn_count);
  return _spawn_positions[spawn];
}

} // namespace bm
