// Copyright (c) 2015 Blowmorph Team

#include "server/activator.h"

#include <Box2D/Box2D.h>

#include "base/body.h"
#include "base/error.h"
#include "base/macros.h"
#include "base/protocol.h"
#include "base/pstdint.h"

#include "server/controller.h"
#include "server/entity.h"

namespace bm {

Activator::Activator(
  Controller* controller,
  uint32_t id,
  const b2Vec2& position,
  Type type
) : Entity(controller, id, "door", position,
           Entity::FILTER_WALL, Entity::FILTER_ALL) {
  // FIXME(xairy): entity config depends on entity type.
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
void Activator::Collide(Bullet* other) {
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
