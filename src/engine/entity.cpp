// Copyright (c) 2015 Blowmorph Team

#include "engine/entity.h"

#include <cmath>

#include <memory>
#include <string>

#include <Box2D/Box2D.h>

#include "base/error.h"
#include "base/macros.h"
#include "base/protocol.h"
#include "base/pstdint.h"

#include "engine/body.h"
#include "engine/config.h"

namespace bm {

Entity::Entity(
	b2World* world,
  uint32_t id,
  Type type,
  const std::string& entity_name,
  b2Vec2 position,
  uint16_t collision_category,
  uint16_t collision_mask
) : id_(id),
    type_(type),
    body_(NULL)
{
  std::string body_name;
  switch (type) {
    case TYPE_ACTIVATOR: {
      auto config = Config::GetInstance()->GetActivatorsConfig();
      CHECK(config.count(entity_name) == 1);
      body_name = config.at(entity_name).body_name;
    } break;
    case TYPE_CRITTER: {
      auto config = Config::GetInstance()->GetCrittersConfig();
      CHECK(config.count(entity_name) == 1);
      body_name = config.at(entity_name).body_name;
    } break;
    case TYPE_KIT: {
      auto config = Config::GetInstance()->GetKitsConfig();
      CHECK(config.count(entity_name) == 1);
      body_name = config.at(entity_name).body_name;
    } break;
    case TYPE_PLAYER: {
      auto config = Config::GetInstance()->GetPlayersConfig();
      CHECK(config.count(entity_name) == 1);
      body_name = config.at(entity_name).body_name;
    } break;
    case TYPE_PROJECTILE: {
      auto config = Config::GetInstance()->GetProjectilesConfig();
      CHECK(config.count(entity_name) == 1);
      body_name = config.at(entity_name).body_name;
    } break;
    case TYPE_WALL: {
      auto config = Config::GetInstance()->GetWallsConfig();
      CHECK(config.count(entity_name) == 1);
      body_name = config.at(entity_name).body_name;
    } break;
    default:
      CHECK(false);  // Unreachable.
  }

  body_ = new Body();
  CHECK(body_ != NULL);
  body_->Create(world, body_name);
  body_->SetUserData(this);
  body_->SetPosition(position);
  body_->SetCollisionFilter(collision_category, collision_mask);
}

Entity::~Entity() {
  if (body_ != NULL) {
    delete body_;
    body_ = NULL;
  }
}

uint32_t Entity::GetId() const {
  return id_;
}

Entity::Type Entity::GetType() const {
  return type_;
}

bool Entity::IsStatic() const {
  if (type_ == TYPE_ACTIVATOR || type_ == TYPE_KIT || type_ == TYPE_WALL) {
    return true;
  }
  return false;
}

b2Vec2 Entity::GetPosition() const {
  return body_->GetPosition();
}
void Entity::SetPosition(const b2Vec2& position) {
  body_->SetPosition(position);
}

float Entity::GetRotation() const {
  return body_->GetRotation();
}
void Entity::SetRotation(float angle) {
  body_->SetRotation(angle);
}

b2Vec2 Entity::GetVelocity() const {
  return body_->GetVelocity();
}

void Entity::SetVelocity(const b2Vec2& velocity) {
  body_->SetVelocity(velocity);
}

float Entity::GetMass() const {
  return body_->GetMass();
}

void Entity::ApplyImpulse(const b2Vec2& impulse) {
  body_->ApplyImpulse(impulse);
}

void Entity::SetImpulse(const b2Vec2& impulse) {
  body_->SetImpulse(impulse);
}

}  // namespace bm
