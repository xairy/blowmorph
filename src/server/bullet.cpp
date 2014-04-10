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

  // !FIXME: load radius from cfg.
  bullet->body_ = CreateCircle(world_manager->GetWorld(), start, 5.0, true);
  b2Vec2 velocity = end - start;
  velocity.Normalize();
  velocity *= speed;
  bullet->body_->SetLinearVelocity(velocity);

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

void Bullet::OnEntityAppearance(Entity* entity) {
}
void Bullet::OnEntityDisappearance(Entity* entity) {
}

void Bullet::Damage(int damage) {
  Destroy();
}

void Bullet::Explode() {
  if (!IsDestroyed()) {
    bool rv = _world_manager->Blow(body_->GetPosition());
    // TODO(xairy): handle error.
    CHECK(rv == true);
    Destroy();
  }
}

// Double dispatch. Collision detection.

bool Bullet::Collide(Entity* entity) {
  return entity->Collide(this);
}

bool Bullet::Collide(Player* other) {
  return Entity::Collide(other, this);
}
bool Bullet::Collide(Dummy* other) {
  return Entity::Collide(other, this);
}
bool Bullet::Collide(Bullet* other) {
  return Entity::Collide(this, other);
}
bool Bullet::Collide(Wall* other) {
  return Entity::Collide(other, this);
}
bool Bullet::Collide(Station* other) {
  return Entity::Collide(other, this);
}

Bullet::Bullet(WorldManager* world_manager, uint32_t id)
  : Entity(world_manager, id) { }

}  // namespace bm
