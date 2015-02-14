// Copyright (c) 2015 Blowmorph Team

#include "server/bullet.h"

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

namespace bm {

Bullet::Bullet(
  Controller* controller,
  uint32_t id,
  uint32_t owner_id,
  const b2Vec2& start,
  const b2Vec2& end,
  Type type
) : Entity(controller, id, "bullet", start, Entity::FILTER_BULLET,
           Entity::FILTER_ALL & ~Entity::FILTER_KIT) {
  SettingsManager* settings = controller->GetSettings();
  float speed = settings->GetFloat("bullet.speed");

  b2Vec2 velocity = end - start;
  velocity.Normalize();
  velocity *= speed;
  body_->ApplyImpulse(body_->GetMass() * velocity);

  _owner_id = owner_id;
  _type = type;
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
  output->angle = body_->GetRotation();
  if (_type == TYPE_ROCKET) {
    output->data[0] = EntitySnapshot::BULLET_TYPE_ROCKET;
  } else if (_type == TYPE_SLIME) {
    output->data[0] = EntitySnapshot::BULLET_TYPE_SLIME;
  } else {
    CHECK(false);
  }
}

void Bullet::Damage(int damage, uint32_t source_id) {
  Destroy();
}

uint32_t Bullet::GetOwnerId() const {
  return _owner_id;
}

Bullet::Type Bullet::GetBulletType() const {
  return _type;
}

// Double dispatch. Collision detection.

void Bullet::Collide(Entity* entity) {
  entity->Collide(this);
}

void Bullet::Collide(Player* other) {
  Entity::Collide(other, this);
}
void Bullet::Collide(Critter* other) {
  Entity::Collide(other, this);
}
void Bullet::Collide(Bullet* other) {
  Entity::Collide(this, other);
}
void Bullet::Collide(Wall* other) {
  Entity::Collide(other, this);
}
void Bullet::Collide(Kit* other) {
  Entity::Collide(other, this);
}
void Bullet::Collide(Activator* other) {
  Entity::Collide(other, this);
}

}  // namespace bm
