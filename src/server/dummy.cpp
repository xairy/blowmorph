// Copyright (c) 2013 Blowmorph Team

#include "server/dummy.h"

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

Dummy::Dummy(
  WorldManager* world_manager,
  uint32_t id,
  const b2Vec2& position,
  int64_t time
) : Entity(world_manager, id, "dummy", position, true,
           Entity::FILTER_BULLET, Entity::FILTER_ALL & ~Entity::FILTER_KIT) {
  SettingsManager* settings = world_manager->GetSettings();
  _speed = settings->GetFloat("dummy.speed");
  _meat = NULL;
}

Dummy::~Dummy() { }

Entity::Type Dummy::GetType() {
  return Entity::TYPE_DUMMY;
}

bool Dummy::IsStatic() {
  return false;
}

void Dummy::Update(int64_t time) {
  if (_meat != NULL) {
    b2Vec2 velocity = _meat->GetPosition() - GetPosition();
    velocity.Normalize();
    velocity *= _speed;
    body_->SetLinearVelocity(velocity);
  }
}

void Dummy::GetSnapshot(int64_t time, EntitySnapshot* output) {
  output->type = EntitySnapshot::ENTITY_TYPE_DUMMY;
  output->time = time;
  output->id = _id;
  output->x = body_->GetPosition().x;
  output->y = body_->GetPosition().y;
}

void Dummy::OnEntityAppearance(Entity* entity) {
  if (entity->GetType() == Entity::TYPE_PLAYER) {
    if (_meat == NULL) {
      _meat = entity;
    } else {
      float current_distance = (_meat->GetPosition() - GetPosition()).Length();
      float new_distance = (entity->GetPosition() - GetPosition()).Length();
      if (new_distance < current_distance) {
        _meat = entity;
      }
    }
  }
}

void Dummy::OnEntityDisappearance(Entity* entity) {
  if (_meat == entity) {
    _meat = NULL;
  }
}

void Dummy::Damage(int damage) {
  Destroy();
}

void Dummy::Explode() {
  if (!IsDestroyed()) {
    _world_manager->Blow(body_->GetPosition());
    Destroy();
  }
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
