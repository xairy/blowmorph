// Copyright (c) 2013 Blowmorph Team

#include "server/bullet.h"

#include <memory>
#include <string>

#include <Box2D/Box2D.h>

#include "base/error.h"
#include "base/macros.h"
#include "base/protocol.h"
#include "base/pstdint.h"
#include "base/settings_manager.h"

#include "server/box2d_utils.h"
#include "server/world_manager.h"

namespace bm {

Bullet::Bullet(
  WorldManager* world_manager,
  uint32_t id,
  uint32_t owner_id,
  const b2Vec2& start,
  const b2Vec2& end
) : Entity(world_manager, id, "bullet", start, true,
           Entity::FILTER_BULLET, Entity::FILTER_ALL & ~Entity::FILTER_KIT) {
  SettingsManager* settings = world_manager->GetSettings();
  float speed = settings->GetFloat("bullet.speed");

  b2Vec2 velocity = end - start;
  velocity.Normalize();
  velocity *= speed;
  body_->SetLinearVelocity(velocity);

  _owner_id = owner_id;
}

Bullet::~Bullet() { }

Entity::Type Bullet::GetType() {
  return Entity::TYPE_BULLET;
}
bool Bullet::IsStatic() {
  return false;
}

void Bullet::GetSnapshot(int64_t time, EntitySnapshot* output) {
  output->type = EntitySnapshot::ENTITY_TYPE_BULLET;
  output->time = time;
  output->id = _id;
  output->x = body_->GetPosition().x;
  output->y = body_->GetPosition().y;
}

void Bullet::Damage(int damage, uint32_t source_id) {
  Destroy();
}

uint32_t Bullet::GetOwnerId() {
  return _owner_id;
}

// Double dispatch. Collision detection.

void Bullet::Collide(Entity* entity) {
  entity->Collide(this);
}

void Bullet::Collide(Player* other) {
  Entity::Collide(other, this);
}
void Bullet::Collide(Dummy* other) {
  Entity::Collide(other, this);
}
void Bullet::Collide(Bullet* other) {
  Entity::Collide(this, other);
}
void Bullet::Collide(Wall* other) {
  Entity::Collide(other, this);
}
void Bullet::Collide(Station* other) {
  Entity::Collide(other, this);
}

}  // namespace bm
