// Copyright (c) 2015 Blowmorph Team

#include "server/critter.h"

#include <memory>
#include <string>

#include <Box2D/Box2D.h>

#include "base/config_reader.h"
#include "base/error.h"
#include "base/macros.h"
#include "base/protocol.h"
#include "base/pstdint.h"

#include "engine/body.h"

#include "server/controller.h"
#include "server/entity.h"

namespace bm {

Critter::Critter(
  Controller* controller,
  uint32_t id,
  const b2Vec2& position,
  const std::string& config_name
) : Entity(controller, id,
           controller->GetCritterSettings()->GetString(config_name + ".body"),
           position, Entity::FILTER_CRITTER,
           Entity::FILTER_ALL & ~Entity::FILTER_KIT) {
  ConfigReader* entity_settings = controller->GetCritterSettings();
  _speed = entity_settings->GetFloat(config_name + ".speed");
  _target = NULL;
  std::string type_name = entity_settings->GetString(config_name + ".type");
  if (type_name == "zombie") {
    type_ = TYPE_ZOMBIE;
  } else {
    CHECK(false);  // Unreachable.
  }
}

Critter::~Critter() { }

Entity::Type Critter::GetType() {
  return Entity::TYPE_CRITTER;
}

bool Critter::IsStatic() {
  return false;
}

void Critter::GetSnapshot(int64_t time, EntitySnapshot* output) {
  output->type = EntitySnapshot::ENTITY_TYPE_CRITTER;
  output->time = time;
  output->id = _id;
  output->x = body_->GetPosition().x;
  output->y = body_->GetPosition().y;
  output->angle = body_->GetRotation();
}

void Critter::Damage(int damage, uint32_t source_id) {
  Destroy();
}

float Critter::GetSpeed() const {
  return _speed;
}

Entity* Critter::GetTarget() const {
  return _target;
}

void Critter::SetTarget(Entity* target) {
  _target = target;
}

void Critter::Collide(Entity* entity) {
  entity->Collide(this);
}

void Critter::Collide(Player* other) {
  Entity::Collide(other, this);
}
void Critter::Collide(Critter* other) {
  Entity::Collide(other, this);
}
void Critter::Collide(Projectile* other) {
  Entity::Collide(this, other);
}
void Critter::Collide(Wall* other) {
  Entity::Collide(other, this);
}
void Critter::Collide(Kit* other) {
  Entity::Collide(other, this);
}
void Critter::Collide(Activator* other) {
  Entity::Collide(other, this);
}

}  // namespace bm
