// Copyright (c) 2015 Blowmorph Team

#include "server/world.h"

#include <map>
#include <vector>
#include <string>

#include <Box2D/Box2D.h>
#include <pugixml.hpp>

#include "base/error.h"
#include "base/pstdint.h"

#include "server/entity.h"
#include "server/id_manager.h"
#include "server/controller.h"

#include "server/activator.h"
#include "server/projectile.h"
#include "server/critter.h"
#include "server/kit.h"
#include "server/player.h"
#include "server/wall.h"

namespace bm {

World::World(Controller* controller, IdManager* id_manager)
    : world_(b2Vec2(0.0f, 0.0f)),
      controller_(controller),
      id_manager_(id_manager) { }

World::~World() {
  std::map<uint32_t, Entity*>::iterator i, end;
  end = static_entities_.end();
  for (i = static_entities_.begin(); i != end; ++i) {
    delete i->second;
  }
  end = dynamic_entities_.end();
  for (i = dynamic_entities_.begin(); i != end; ++i) {
    delete i->second;
  }
}

b2World* World::GetBox2DWorld() {
  return &world_;
}

float World::GetBound() const {
  return bound_;
}

float World::GetBlockSize() const {
  return block_size_;
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

Activator* World::CreateActivator(
  const b2Vec2& position,
  const std::string& entity_name
) {
  uint32_t id = id_manager_->NewId();
  Activator* activator = new Activator(controller_, id, position, entity_name);
  CHECK(activator != NULL);
  AddEntity(id, activator);
  return activator;
}

Critter* World::CreateCritter(
  const b2Vec2& position,
  const std::string& entity_name
) {
  uint32_t id = id_manager_->NewId();
  Critter* critter = new Critter(controller_, id, position, entity_name);
  CHECK(critter != NULL);
  AddEntity(id, critter);
  return critter;
}

Kit* World::CreateKit(
  const b2Vec2& position,
  int health_regeneration,
  int energy_regeneration,
  const std::string& entity_name
) {
  uint32_t id = id_manager_->NewId();
  Kit* kit = new Kit(controller_, id, position,
      health_regeneration, energy_regeneration, entity_name);
  CHECK(kit != NULL);
  AddEntity(id, kit);
  return kit;
}

Player* World::CreatePlayer(
    const b2Vec2& position,
    const std::string& entity_name
) {
  uint32_t id = id_manager_->NewId();
  Player* player = new Player(controller_, entity_name, id, position);
  CHECK(player != NULL);
  AddEntity(id, player);
  return player;
}

Projectile* World::CreateProjectile(
  uint32_t owner_id,
  const b2Vec2& start,
  const b2Vec2& end,
  const std::string& entity_name
) {
  CHECK(static_entities_.count(owner_id) +
    dynamic_entities_.count(owner_id) == 1);
  uint32_t id = id_manager_->NewId();
  Projectile* projectile = new Projectile(controller_,
    id, owner_id, start, end, entity_name);
  CHECK(projectile != NULL);
  AddEntity(id, projectile);
  return projectile;
}

Wall* World::CreateWall(
  const b2Vec2& position,
  const std::string& entity_name
) {
  uint32_t id = id_manager_->NewId();
  Wall* wall = new Wall(controller_, id, position, entity_name);
  CHECK(wall != NULL);
  AddEntity(id, wall);
  return wall;
}

void World::AddEntity(uint32_t id, Entity* entity) {
  CHECK(static_entities_.count(id) == 0 && dynamic_entities_.count(id) == 0);
  if (entity->IsStatic()) {
    static_entities_[id] = entity;
  } else {
    dynamic_entities_[id] = entity;
  }
  // OnEntityAppearance(entity); !refactor
}

std::vector<b2Vec2>* World::GetSpawnPositions() {
  return &spawn_positions_;
}

bool World::LoadMap(const std::string& file) {
  pugi::xml_document document;
  pugi::xml_parse_result parse_result = document.load_file(file.c_str());
  if (!parse_result) {
    Error::Throw(__FILE__, __LINE__, "Unable to parse %s!\n", file.c_str());
    return false;
  }
  pugi::xml_node map_node = document.child("map");
  if (!map_node) {
    Error::Throw(__FILE__, __LINE__,
      "Tag 'map' not found in %s!\n", file.c_str());
    return false;
  }

  pugi::xml_attribute block_size = map_node.attribute("block_size");
  if (!block_size) {
    Error::Throw(__FILE__, __LINE__,
      "Tag 'map' does not have attribute 'block_size' in %s!\n", file.c_str());
    return false;
  }
  block_size_ = block_size.as_float();

  pugi::xml_attribute bound = map_node.attribute("bound");
  if (!bound) {
    Error::Throw(__FILE__, __LINE__,
      "Tag 'map' does not have attribute 'bound' in %s!\n", file.c_str());
    return false;
  }
  bound_ = bound.as_float();

  pugi::xml_node node;
  for (node = map_node.first_child(); node; node = node.next_sibling()) {
    if (std::string(node.name()) == "wall") {
      if (!LoadWall(node)) {
        return false;
      }
    } else if (std::string(node.name()) == "chunk") {
      if (!LoadChunk(node)) {
        return false;
      }
    } else if (std::string(node.name()) == "spawn") {
      if (!LoadSpawn(node)) {
        return false;
      }
    } else if (std::string(node.name()) == "kit") {
      if (!LoadKit(node)) {
        return false;
      }
    }
  }

  return true;
}

bool World::LoadWall(const pugi::xml_node& node) {
  CHECK(std::string(node.name()) == "wall");

  pugi::xml_attribute x = node.attribute("x");
  pugi::xml_attribute y = node.attribute("y");
  pugi::xml_attribute type = node.attribute("type");
  if (!x || !y || !type) {
    THROW_ERROR("Incorrect format of 'wall' in map file!\n");
    return false;
  } else {
    CHECK(std::string(type.name()) == "type");
    CreateWall(b2Vec2(x.as_int() * block_size_, y.as_int() * block_size_),
        std::string(type.value()) + "_wall");
  }

  return true;
}

bool World::LoadChunk(const pugi::xml_node& node) {
  CHECK(std::string(node.name()) == "chunk");

  pugi::xml_attribute x = node.attribute("x");
  pugi::xml_attribute y = node.attribute("y");
  pugi::xml_attribute width = node.attribute("width");
  pugi::xml_attribute height = node.attribute("height");
  pugi::xml_attribute type = node.attribute("type");
  if (!x || !y || !width || !height || !type) {
    THROW_ERROR("Incorrect format of 'chunk' in map file!\n");
    return false;
  } else {
    int xv = x.as_int();
    int yv = y.as_int();
    int wv = width.as_int();
    int hv = height.as_int();
    CHECK(std::string(type.name()) == "type");
    for (int i = 0; i < wv; i++) {
      for (int j = 0; j < hv; j++) {
        CreateWall(b2Vec2((xv + i) * block_size_, (yv + j) * block_size_),
            std::string(type.value()) + "_wall");
      }
    }
  }

  return true;
}

bool World::LoadSpawn(const pugi::xml_node& node) {
  CHECK(std::string(node.name()) == "spawn");

  pugi::xml_attribute x_attr = node.attribute("x");
  pugi::xml_attribute y_attr = node.attribute("y");
  if (!x_attr || !y_attr) {
    THROW_ERROR("Incorrect format of 'spawn' in map file!\n");
    return false;
  } else {
    float x = x_attr.as_float();
    float y = y_attr.as_float();
    spawn_positions_.push_back(b2Vec2(x, y));
  }

  return true;
}

bool World::LoadKit(const pugi::xml_node& node) {
  CHECK(std::string(node.name()) == "kit");

  pugi::xml_attribute x_attr = node.attribute("x");
  pugi::xml_attribute y_attr = node.attribute("y");
  pugi::xml_attribute hr_attr = node.attribute("health_regeneration");
  pugi::xml_attribute er_attr = node.attribute("energy_regeneration");
  pugi::xml_attribute type_attr = node.attribute("type");
  if (!x_attr || !y_attr || !hr_attr || !er_attr || !type_attr) {
    THROW_ERROR("Incorrect format of 'kit' in map file!\n");
    return false;
  } else {
    float x = x_attr.as_float();
    float y = y_attr.as_float();
    int hr = hr_attr.as_int();
    int er = er_attr.as_int();
    CHECK(std::string(type_attr.name()) == "type");
    CreateKit(b2Vec2(x, y), hr, er, std::string(type_attr.value()) + "_kit");
  }

  return true;
}

}  // namespace bm
