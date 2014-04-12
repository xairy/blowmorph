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

Bullet* Bullet::Create(
  WorldManager* world_manager,
  uint32_t id,
  uint32_t owner_id,
  const b2Vec2& start,
  const b2Vec2& end,
  int64_t time
) {
  SettingsManager* settings = world_manager->GetSettings();
  float speed = settings->GetFloat("bullet.speed");

  Bullet* bullet = new Bullet(world_manager, id);
  CHECK(bullet != NULL);

  b2World* world = world_manager->GetWorld();
  b2Body* body = CreateBody(world, settings, "bullet.shape", true);
  SetBodyPosition(body, start);
  body->SetUserData(bullet);
  SetCollisionFilter(body, Entity::FILTER_BULLET,
      Entity::FILTER_ALL & ~Entity::FILTER_KIT);

  b2Vec2 velocity = end - start;
  velocity.Normalize();
  velocity *= speed;
  body->SetLinearVelocity(velocity);

  bullet->body_ = body;
  bullet->_owner_id = owner_id;

  return bullet;
}

Bullet::~Bullet() { }

std::string Bullet::GetType() {
  return "Bullet";
}
bool Bullet::IsStatic() {
  return false;
}

void Bullet::Update(int64_t time) { }

void Bullet::GetSnapshot(int64_t time, EntitySnapshot* output) {
  output->type = EntitySnapshot::ENTITY_TYPE_BULLET;
  output->time = time;
  output->id = _id;
  output->x = body_->GetPosition().x;
  output->y = body_->GetPosition().y;
}

void Bullet::OnEntityAppearance(Entity* entity) { }
void Bullet::OnEntityDisappearance(Entity* entity) { }

void Bullet::Damage(int damage) {
  Destroy();
}

void Bullet::Explode() {
  if (!IsDestroyed()) {
    _world_manager->Blow(body_->GetPosition());
    Destroy();
  }
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

Bullet::Bullet(WorldManager* world_manager, uint32_t id)
  : Entity(world_manager, id) { }

}  // namespace bm
