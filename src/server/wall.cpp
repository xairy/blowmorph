// Copyright (c) 2015 Blowmorph Team

#include "server/wall.h"

#include <memory>
#include <string>

#include <Box2D/Box2D.h>

#include "base/error.h"
#include "base/macros.h"
#include "base/protocol.h"
#include "base/pstdint.h"

#include "engine/body.h"
#include "engine/config.h"

#include "server/controller.h"
#include "server/entity.h"

namespace bm {

Wall::Wall(
  Controller* controller,
  uint32_t id,
  const b2Vec2& position,
  const std::string& entity_name
) : Entity(controller, id, entity_name, Entity::TYPE_WALL, position,
           Entity::FILTER_WALL, Entity::FILTER_ALL) {
  auto config = Config::GetInstance()->GetWallsConfig();
  CHECK(config.count(entity_name) == 1);
  Config::WallConfig::Type type = config.at(entity_name).type;
  if (type == Config::WallConfig::TYPE_ORDINARY) {
    _type = TYPE_ORDINARY;
  } else if (type == Config::WallConfig::TYPE_UNBREAKABLE) {
    _type = TYPE_UNBREAKABLE;
  } else if (type == Config::WallConfig::TYPE_MORPHED) {
    _type = TYPE_MORPHED;
  } else {
    CHECK(false);  // Unreachable.
  }
}

Wall::~Wall() { }

void Wall::GetSnapshot(int64_t time, EntitySnapshot* output) {
  output->type = EntitySnapshot::ENTITY_TYPE_WALL;
  output->time = time;
  output->id = _id;
  output->x = body_->GetPosition().x;
  output->y = body_->GetPosition().y;
  output->angle = body_->GetRotation();
  if (_type == TYPE_ORDINARY) {
    output->data[0] = EntitySnapshot::WALL_TYPE_ORDINARY;
  } else if (_type == TYPE_UNBREAKABLE) {
    output->data[0] = EntitySnapshot::WALL_TYPE_UNBREAKABLE;
  } else if (_type == TYPE_MORPHED) {
    output->data[0] = EntitySnapshot::WALL_TYPE_MORPHED;
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

void Wall::Collide(Entity* entity) {
  entity->Collide(this);
}

void Wall::Collide(Player* other) {
  Entity::Collide(this, other);
}
void Wall::Collide(Critter* other) {
  Entity::Collide(this, other);
}
void Wall::Collide(Projectile* other) {
  Entity::Collide(this, other);
}
void Wall::Collide(Wall* other) {
  Entity::Collide(other, this);
}
void Wall::Collide(Kit* other) {
  Entity::Collide(other, this);
}
void Wall::Collide(Activator* other) {
  Entity::Collide(other, this);
}

}  // namespace bm
