// Copyright (c) 2015 Blowmorph Team

#include "server/door.h"

#include <cmath>

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

Door::Door(
  Controller* controller,
  uint32_t id,
  const b2Vec2& position,
  const std::string& entity_name
) : ServerEntity(controller, id, Entity::TYPE_DOOR, entity_name, position,
                 Entity::FILTER_DOOR, Entity::FILTER_ALL) {
  auto config = Config::GetInstance()->GetDoorsConfig();
  CHECK(config.count(entity_name) == 1);
  activation_distance_ = config.at(entity_name).activation_distance;
  door_closed_ = true;
}

Door::~Door() { }

void Door::GetSnapshot(int64_t time, EntitySnapshot* output) {
  ServerEntity::GetSnapshot(time, output);
  output->type = EntitySnapshot::ENTITY_TYPE_DOOR;
}

void Door::Damage(int damage, uint32_t source_id) { }

float Door::GetActivationDistance() const {
  return activation_distance_;
}

void Door::Activate(Entity* activator) {
  printf("Player %d activated door %d\n", activator->GetId(), GetId());
  if (door_closed_) {
    SetRotation(M_PI / 2);
    door_closed_ = false;
  } else {
    SetRotation(0.0f);
    door_closed_ = true;
  }
  SetUpdatedFlag(true);
}

// Double dispatch. Collision detection.

void Door::Collide(ServerEntity* entity) {
  entity->Collide(this);
}

void Door::Collide(Activator* other) {
  ServerEntity::Collide(this, other);
}
void Door::Collide(Critter* other) {
  ServerEntity::Collide(this, other);
}
void Door::Collide(Door* other) {
  ServerEntity::Collide(this, other);
}
void Door::Collide(Kit* other) {
  ServerEntity::Collide(this, other);
}
void Door::Collide(Player* other) {
  ServerEntity::Collide(this, other);
}
void Door::Collide(Projectile* other) {
  ServerEntity::Collide(this, other);
}
void Door::Collide(Wall* other) {
  ServerEntity::Collide(this, other);
}

}  // namespace bm
