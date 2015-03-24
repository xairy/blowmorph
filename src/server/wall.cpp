// Copyright (c) 2015 Blowmorph Team

#include "server/wall.h"

#include <memory>
#include <string>

#include <Box2D/Box2D.h>

#include "base/error.h"
#include "base/macros.h"
#include "base/pstdint.h"

#include "engine/body.h"
#include "engine/config.h"
#include "engine/protocol.h"

#include "server/controller.h"
#include "server/entity.h"

namespace bm {

Wall::Wall(
  Controller* controller,
  uint32_t id,
  const b2Vec2& position,
  const std::string& entity_name
) : ServerEntity(controller, id, Entity::TYPE_WALL, entity_name, position,
                 Entity::FILTER_WALL, Entity::FILTER_ALL) {
  auto config = Config::GetInstance()->GetWallsConfig();
  CHECK(config.count(entity_name) == 1);
  Config::WallConfig::Type type = config.at(entity_name).type;
  if (type == Config::WallConfig::TYPE_ORDINARY) {
    _type = TYPE_ORDINARY;
  } else if (type == Config::WallConfig::TYPE_UNBREAKABLE) {
    _type = TYPE_UNBREAKABLE;
  } else {
    CHECK(false);  // Unreachable.
  }
}

Wall::~Wall() { }

void Wall::GetSnapshot(int64_t time, EntitySnapshot* output) {
  ServerEntity::GetSnapshot(time, output);
  output->type = EntitySnapshot::ENTITY_TYPE_WALL;
  if (_type == TYPE_ORDINARY) {
    output->data[0] = EntitySnapshot::WALL_TYPE_ORDINARY;
  } else if (_type == TYPE_UNBREAKABLE) {
    output->data[0] = EntitySnapshot::WALL_TYPE_UNBREAKABLE;
  } else {
    CHECK(false);
  }
}

void Wall::Damage(int damage, uint32_t source_id) {
  if (_type != TYPE_UNBREAKABLE) {
    Destroy();
  }
}

// Double dispatch. Collision detection.

void Wall::Collide(ServerEntity* entity) {
  entity->Collide(this);
}

void Wall::Collide(Activator* other) {
  ServerEntity::Collide(other, this);
}
void Wall::Collide(Critter* other) {
  ServerEntity::Collide(this, other);
}
void Wall::Collide(Door* other) {
  ServerEntity::Collide(other, this);
}
void Wall::Collide(Kit* other) {
  ServerEntity::Collide(other, this);
}
void Wall::Collide(Player* other) {
  ServerEntity::Collide(this, other);
}
void Wall::Collide(Projectile* other) {
  ServerEntity::Collide(this, other);
}
void Wall::Collide(Wall* other) {
  ServerEntity::Collide(other, this);
}

}  // namespace bm
