// Copyright (c) 2013 Blowmorph Team

#include "server/wall.h"

#include <memory>
#include <string>

#include <Box2D/Box2D.h>

#include "base/body.h"
#include "base/error.h"
#include "base/macros.h"
#include "base/protocol.h"
#include "base/pstdint.h"
#include "base/settings_manager.h"

#include "server/controller.h"
#include "server/entity.h"

namespace bm {

Wall::Wall(
  Controller* controller,
  uint32_t id,
  const b2Vec2& position,
  Type type
) : Entity(controller, id, TypeToEntityName(type), position,
           Entity::FILTER_WALL, Entity::FILTER_ALL) {
  _type = type;
}

Wall::~Wall() { }

Entity::Type Wall::GetType() {
  return Entity::TYPE_WALL;
}
bool Wall::IsStatic() {
  return true;
}

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
void Wall::Collide(Dummy* other) {
  Entity::Collide(this, other);
}
void Wall::Collide(Bullet* other) {
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

std::string Wall::TypeToEntityName(Wall::Type type) {
  switch (type) {
    case Wall::TYPE_ORDINARY:
      return "ordinary_wall";
    case Wall::TYPE_UNBREAKABLE:
      return "unbreakable_wall";
    case Wall::TYPE_MORPHED:
      return "morphed_wall";
    default:
      CHECK(false);
  }
}

}  // namespace bm
