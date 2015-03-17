// Copyright (c) 2015 Blowmorph Team

#include "server/world.h"

#include <map>
#include <vector>
#include <string>

#include <Box2D/Box2D.h>

#include "base/error.h"
#include "base/id_manager.h"
#include "base/pstdint.h"

#include "engine/map.h"
#include "engine/world.h"

#include "server/entity.h"
#include "server/controller.h"

#include "server/activator.h"
#include "server/critter.h"
#include "server/kit.h"
#include "server/projectile.h"
#include "server/player.h"
#include "server/wall.h"

namespace bm {

ServerWorld::ServerWorld(Controller* controller) : controller_(controller) { }
ServerWorld::~ServerWorld() { }

float ServerWorld::GetBound() const {
  return bound_;
}

float ServerWorld::GetBlockSize() const {
  return block_size_;
}

Activator* ServerWorld::CreateActivator(
  const b2Vec2& position,
  const std::string& entity_name
) {
  uint32_t id = id_manager_.NewId();
  Activator* activator = new Activator(controller_, id, position, entity_name);
  CHECK(activator != NULL);
  AddEntity(id, activator);
  return activator;
}

Critter* ServerWorld::CreateCritter(
  const b2Vec2& position,
  const std::string& entity_name
) {
  uint32_t id = id_manager_.NewId();
  Critter* critter = new Critter(controller_, id, position, entity_name);
  CHECK(critter != NULL);
  AddEntity(id, critter);
  return critter;
}

Kit* ServerWorld::CreateKit(
  const b2Vec2& position,
  const std::string& entity_name
) {
  uint32_t id = id_manager_.NewId();
  Kit* kit = new Kit(controller_, id, position, entity_name);
  CHECK(kit != NULL);
  AddEntity(id, kit);
  return kit;
}

Player* ServerWorld::CreatePlayer(
    const b2Vec2& position,
    const std::string& entity_name
) {
  uint32_t id = id_manager_.NewId();
  Player* player = new Player(controller_, entity_name, id, position);
  CHECK(player != NULL);
  AddEntity(id, player);
  return player;
}

Projectile* ServerWorld::CreateProjectile(
  uint32_t owner_id,
  const b2Vec2& start,
  const b2Vec2& end,
  const std::string& entity_name
) {
  CHECK(GetEntity(owner_id) != NULL);
  uint32_t id = id_manager_.NewId();
  Projectile* projectile = new Projectile(controller_,
    id, owner_id, start, end, entity_name);
  CHECK(projectile != NULL);
  AddEntity(id, projectile);
  return projectile;
}

Wall* ServerWorld::CreateWall(
  const b2Vec2& position,
  const std::string& entity_name
) {
  uint32_t id = id_manager_.NewId();
  Wall* wall = new Wall(controller_, id, position, entity_name);
  CHECK(wall != NULL);
  AddEntity(id, wall);
  return wall;
}

std::vector<b2Vec2>* ServerWorld::GetSpawnPositions() {
  return &spawn_positions_;
}

bool ServerWorld::LoadMap(const std::string& file) {
  Map map;
  if (!map.Load(file)) {
    REPORT_ERROR("Can't load map '%s'.", file.c_str());
    return false;
  }

  block_size_ = map.GetBlockSize();
  bound_ = (map.GetSize() + 1) * block_size_;

  for (auto spawn : map.GetSpawns()) {
    float x = spawn.x * block_size_;
    float y = spawn.y * block_size_;
    spawn_positions_.push_back(b2Vec2(x, y));
  }

  for (auto wall : map.GetWalls()) {
    float x = wall.x * block_size_;
    float y = wall.y * block_size_;
    CreateWall(b2Vec2(x, y), wall.entity_name);
  }

  for (auto kit : map.GetKits()) {
    float x = kit.x * block_size_;
    float y = kit.y * block_size_;
    CreateKit(b2Vec2(x, y), kit.entity_name);
  }

  return true;
}

}  // namespace bm
