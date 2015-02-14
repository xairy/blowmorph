// Copyright (c) 2015 Blowmorph Team

#include "server/critter.h"

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

Critter::Critter(
  Controller* controller,
  uint32_t id,
  const b2Vec2& position
) : Entity(controller, id, "zombie", position, Entity::FILTER_BULLET,
           Entity::FILTER_ALL & ~Entity::FILTER_KIT) {
  SettingsManager* settings = controller->GetEntitySettings();
  _speed = settings->GetFloat("zombie.speed");
  _target = NULL;
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
void Critter::Collide(Bullet* other) {
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
