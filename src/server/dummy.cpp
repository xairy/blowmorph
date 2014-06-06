// Copyright (c) 2013 Blowmorph Team

#include "server/dummy.h"

#include <memory>
#include <string>

#include <Box2D/Box2D.h>

#include "base/body.h"
#include "base/error.h"
#include "base/macros.h"
#include "base/protocol.h"
#include "base/pstdint.h"
#include "base/settings_manager.h"

#include "server/world_manager.h"

namespace bm {

Dummy::Dummy(
  WorldManager* world_manager,
  uint32_t id,
  const b2Vec2& position
) : Entity(world_manager, id, "dummy", position, true,
           Entity::FILTER_BULLET, Entity::FILTER_ALL & ~Entity::FILTER_KIT) {
  SettingsManager* settings = world_manager->GetSettings();
  _speed = settings->GetFloat("dummy.speed");
  _target = NULL;
}

Dummy::~Dummy() { }

Entity::Type Dummy::GetType() {
  return Entity::TYPE_DUMMY;
}

bool Dummy::IsStatic() {
  return false;
}

void Dummy::GetSnapshot(int64_t time, EntitySnapshot* output) {
  output->type = EntitySnapshot::ENTITY_TYPE_DUMMY;
  output->time = time;
  output->id = _id;
  output->x = body_->GetPosition().x;
  output->y = body_->GetPosition().y;
}

void Dummy::Damage(int damage, uint32_t source_id) {
  Destroy();
}

float Dummy::GetSpeed() const {
  return _speed;
}

Entity* Dummy::GetTarget() const {
  return _target;
}

void Dummy::SetTarget(Entity* target) {
  _target = target;
}

void Dummy::Collide(Entity* entity) {
  entity->Collide(this);
}

void Dummy::Collide(Player* other) {
  Entity::Collide(other, this);
}
void Dummy::Collide(Dummy* other) {
  Entity::Collide(other, this);
}
void Dummy::Collide(Bullet* other) {
  Entity::Collide(this, other);
}
void Dummy::Collide(Wall* other) {
  Entity::Collide(other, this);
}
void Dummy::Collide(Station* other) {
  Entity::Collide(other, this);
}

}  // namespace bm
