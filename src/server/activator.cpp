// Copyright (c) 2015 Blowmorph Team

#include "server/activator.h"

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

Activator::Activator(
  Controller* controller,
  uint32_t id,
  const b2Vec2& position,
  const std::string& entity_name
) : ServerEntity(controller, id, Entity::TYPE_ACTIVATOR, entity_name, position,
                 Entity::FILTER_WALL, Entity::FILTER_ALL) {
  auto config = Config::GetInstance()->GetActivatorsConfig();
  CHECK(config.count(entity_name) == 1);
  activation_distance_ = config.at(entity_name).activation_distance;
  Config::ActivatorConfig::Type type = config.at(entity_name).type;
  if (type == Config::ActivatorConfig::TYPE_DOOR) {
    type_ = TYPE_DOOR;
    door_closed_ = true;
  } else {
    CHECK(false);  // Unreachable.
  }
}

Activator::~Activator() { }

void Activator::GetSnapshot(int64_t time, EntitySnapshot* output) {
  ServerEntity::GetSnapshot(time, output);
  output->type = EntitySnapshot::ENTITY_TYPE_ACTIVATOR;
  // FIXME(xairy): set data[0] depending on activator type.
}

void Activator::Damage(int damage, uint32_t source_id) { }

float Activator::GetActivationDistance() const {
  return activation_distance_;
}

void Activator::Activate(Entity* activator) {
  printf("Player %d activated %d\n", activator->GetId(), GetId());
  if (type_ == TYPE_DOOR) {
    if (door_closed_) {
      SetPosition(GetPosition() + b2Vec2(37.5f, 37.5f));
      SetRotation(M_PI / 2);
      door_closed_ = false;
    } else {
      SetPosition(GetPosition() - b2Vec2(37.5f, 37.5f));
      SetRotation(0.0f);
      door_closed_ = true;
    }
    SetUpdatedFlag(true);
  }
}

// Double dispatch. Collision detection.

void Activator::Collide(ServerEntity* entity) {
  entity->Collide(this);
}

void Activator::Collide(Player* other) {
  ServerEntity::Collide(this, other);
}
void Activator::Collide(Critter* other) {
  ServerEntity::Collide(this, other);
}
void Activator::Collide(Projectile* other) {
  ServerEntity::Collide(this, other);
}
void Activator::Collide(Wall* other) {
  ServerEntity::Collide(this, other);
}
void Activator::Collide(Kit* other) {
  ServerEntity::Collide(this, other);
}
void Activator::Collide(Activator* other) {
  ServerEntity::Collide(this, other);
}

}  // namespace bm
