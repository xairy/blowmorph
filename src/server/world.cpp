// Copyright (c) 2015 Blowmorph Team

#include "server/world.h"

#include <fstream>
#include <map>
#include <vector>
#include <string>

#include <Box2D/Box2D.h>

#include "base/error.h"
#include "base/id_manager.h"
#include "base/json.h"
#include "base/pstdint.h"

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
  std::fstream stream(file);
  if (!stream.is_open()) {
    THROW_ERROR("Can't open file '%s'.", file.c_str());
    return false;
  }

  Json::Value root;
  Json::Reader reader;

  bool success = reader.parse(stream, root, false);
  if (!success) {
      std::string error = reader.getFormatedErrorMessages();
      THROW_ERROR("Can't parse '%s':\n%s", file.c_str(), error.c_str());
      return false;
  }

  // Load globals.

  if (!GetFloat32(root["block_size"], &block_size_)) {
    THROW_ERROR("Config '%s' of type '%s' not found in '%s'.",
        "block_size", "float", file.c_str());
    return false;
  }

  if (!GetFloat32(root["bound"], &bound_)) {
    THROW_ERROR("Config '%s' of type '%s' not found in '%s'.",
        "bound", "float", file.c_str());
    return false;
  }

  // Load spawns.

  Json::Value spawns = root["spawns"];
  if (spawns == Json::Value::null) {
    THROW_ERROR("Config '%s' of type '%s' not found in '%s'.",
        "spawns", "array", file.c_str());
    return false;
  }
  if (spawns.size() == 0) {
    THROW_ERROR("Array '%s' is empty in '%s'.", "spawns", file.c_str());
    return false;
  }

  for (int i = 0; i < spawns.size(); i++) {
    b2Vec2 spawn;
    if (!GetFloat32(spawns[i]["x"], &spawn.x)) {
      THROW_ERROR("Config 'spawns[%d].x' of type '%s' not found in '%s'.",
          i, "float", file.c_str());
      return false;
    }
    if (!GetFloat32(spawns[i]["y"], &spawn.y)) {
      THROW_ERROR("Config 'spawns[%d].y' of type '%s' not found in '%s'.",
          i, "float", file.c_str());
      return false;
    }
    spawn_positions_.push_back(spawn);
  }

  // Load walls.

  Json::Value chunks = root["chunks"];
  if (chunks == Json::Value::null) {
    THROW_ERROR("Config '%s' of type '%s' not found in '%s'.",
        "chunks", "array", file.c_str());
    return false;
  }
  if (chunks.size() == 0) {
    THROW_ERROR("Array '%s' is empty in '%s'.", "chunks", file.c_str());
    return false;
  }

  for (int i = 0; i < chunks.size(); i++) {
    int x, y;
    int width, height;
    std::string entity;

    if (!GetInt32(chunks[i]["x"], &x)) {
      THROW_ERROR("Config 'chunks[%d].x' of type '%s' not found in '%s'.",
          i, "int", file.c_str());
      return false;
    }
    if (!GetInt32(chunks[i]["y"], &y)) {
      THROW_ERROR("Config 'chunks[%d].y' of type '%s' not found in '%s'.",
          i, "int", file.c_str());
      return false;
    }
    if (!GetInt32(chunks[i]["width"], &width)) {
      THROW_ERROR("Config 'chunks[%d].width' of type '%s' not found in '%s'.",
          i, "int", file.c_str());
      return false;
    }
    if (!GetInt32(chunks[i]["height"], &height)) {
      THROW_ERROR("Config 'chunks[%d].height' of type '%s' not found in '%s'.",
          i, "int", file.c_str());
      return false;
    }
    if (!GetString(chunks[i]["entity"], &entity)) {
      THROW_ERROR("Config 'chunks[%d].entity' of type '%s' not found in '%s'.",
          i, "string", file.c_str());
      return false;
    }

    for (int i = 0; i < width; i++) {
      for (int j = 0; j < height; j++) {
        CreateWall(b2Vec2((x + i) * block_size_, (y + j) * block_size_),
            entity);
      }
    }
  }

  // Load kits.

  Json::Value kits = root["kits"];
  if (kits == Json::Value::null) {
    THROW_ERROR("Config '%s' of type '%s' not found in '%s'.",
        "kits", "array", file.c_str());
    return false;
  }
  if (kits.size() == 0) {
    THROW_ERROR("Array '%s' is empty in '%s'.", "kits", file.c_str());
    return false;
  }

  for (int i = 0; i < kits.size(); i++) {
    float x, y;
    std::string entity;

    if (!GetFloat32(kits[i]["x"], &x)) {
      THROW_ERROR("Config 'kits[%d].x' of type '%s' not found in '%s'.",
          i, "float", file.c_str());
      return false;
    }
    if (!GetFloat32(kits[i]["y"], &y)) {
      THROW_ERROR("Config 'kits[%d].y' of type '%s' not found in '%s'.",
          i, "float", file.c_str());
      return false;
    }
    if (!GetString(kits[i]["entity"], &entity)) {
      THROW_ERROR("Config 'kits[%d].entity' of type '%s' not found in '%s'.",
          i, "string", file.c_str());
      return false;
    }

    CreateKit(b2Vec2(x, y), entity);
  }

  return true;
}

}  // namespace bm
