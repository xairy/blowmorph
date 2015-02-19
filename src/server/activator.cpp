// Copyright (c) 2015 Blowmorph Team

#include "server/activator.h"

#include <string>

#include <Box2D/Box2D.h>

#include "base/error.h"
#include "base/macros.h"
#include "base/protocol.h"
#include "base/pstdint.h"

#include "engine/body.h"

#include "server/controller.h"
#include "server/entity.h"

namespace bm {

Activator::Activator(
  Controller* controller,
  uint32_t id,
  const b2Vec2& position,
  const std::string& config_name
) : Entity(controller, id,
           controller->GetActivatorSettings()->GetString(config_name + ".body"),
           position, Entity::FILTER_WALL, Entity::FILTER_ALL) {
  SettingsManager* settings = controller->GetActivatorSettings();
  activation_distance_ = settings->GetFloat(
      config_name + ".activation_distance");
  std::string type_name = settings->GetString(config_name + ".type");
  if (type_name == "door") {
    type_ = TYPE_DOOR;
  } else {
    CHECK(false);  // Unreachable.
  }
}

Activator::~Activator() { }

Entity::Type Activator::GetType() {
  return Entity::TYPE_ACTIVATOR;
}

bool Activator::IsStatic() {
  return true;
}

void Activator::GetSnapshot(int64_t time, EntitySnapshot* output) {
  output->type = EntitySnapshot::ENTITY_TYPE_ACTIVATOR;
  output->time = time;
  output->id = _id;
  output->x = body_->GetPosition().x;
  output->y = body_->GetPosition().y;
  output->angle = body_->GetRotation();
  // FIXME(xairy): set data[0] depending on activator type.
}

void Activator::Damage(int damage, uint32_t source_id) { }

float Activator::GetActivationDistance() const {
  return activation_distance_;
}

void Activator::Activate(Entity* activator) {
  printf("Player %d activated %d\n",
      activator->GetId(), GetId());
}

// Double dispatch. Collision detection.

void Activator::Collide(Entity* entity) {
  entity->Collide(this);
}

void Activator::Collide(Player* other) {
  Entity::Collide(this, other);
}
void Activator::Collide(Critter* other) {
  Entity::Collide(this, other);
}
void Activator::Collide(Projectile* other) {
  Entity::Collide(this, other);
}
void Activator::Collide(Wall* other) {
  Entity::Collide(this, other);
}
void Activator::Collide(Kit* other) {
  Entity::Collide(this, other);
}
void Activator::Collide(Activator* other) {
  Entity::Collide(this, other);
}

}  // namespace bm
