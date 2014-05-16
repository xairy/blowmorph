// Copyright (c) 2013 Blowmorph Team

#include "server/bullet.h"

#include <memory>
#include <string>

#include "base/error.h"
#include "base/macros.h"
#include "base/protocol.h"
#include "base/pstdint.h"
#include "base/settings_manager.h"

#include "server/vector.h"
#include "server/shape.h"
#include "server/world_manager.h"

namespace bm {

Bullet* Bullet::Create(
  WorldManager* world_manager,
  uint32_t id,
  uint32_t owner_id,
  const Vector2f& start,
  const Vector2f& end,
  int64_t time
) {
  SettingsManager* settings = world_manager->GetSettings();
  float speed = settings->GetFloat("bullet.speed");

  std::auto_ptr<Bullet> bullet(new Bullet(world_manager, id));
  CHECK(bullet.get() != NULL);

  std::auto_ptr<Shape> shape(world_manager->LoadShape("bullet.shape"));
  if (shape.get() == NULL) {
    return NULL;
  }
  shape->SetPosition(start);

  bullet->_shape = shape.release();
  bullet->_owner_id = owner_id;
  bullet->_start = start;
  bullet->_end = end;
  bullet->_speed = speed;
  bullet->_start_time = time;

  return bullet.release();
}

Bullet::~Bullet() { }

std::string Bullet::GetType() {
  return "Bullet";
}
bool Bullet::IsStatic() {
  return false;
}

void Bullet::Update(int64_t time) {
  CHECK(time >= _start_time);
  Vector2f direction = _end - _start;
  float magnitude = Magnitude(direction);
  if (magnitude != 0.0f) {
    float dt = static_cast<float>(time - _start_time);
    _shape->SetPosition(_start + direction / magnitude * dt * _speed);
  }
}

void Bullet::GetSnapshot(int64_t time, EntitySnapshot* output) {
  output->type = EntitySnapshot::ENTITY_TYPE_BULLET;
  output->time = time;
  output->id = _id;
  output->x = _shape->GetPosition().x;
  output->y = _shape->GetPosition().y;
}

void Bullet::OnEntityAppearance(Entity* entity) {
}
void Bullet::OnEntityDisappearance(Entity* entity) {
}

void Bullet::Damage(int damage, uint32_t source_id) {
  Destroy();
}

void Bullet::Explode(uint32_t source_id) {
  if (!IsDestroyed()) {
    bool rv = _world_manager->Blow(_shape->GetPosition(), source_id);
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
