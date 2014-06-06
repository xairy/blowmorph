// Copyright (c) 2013 Blowmorph Team

#include "server/entity.h"

#include <cmath>

#include <memory>
#include <string>

#include <Box2D/Box2D.h>

#include "base/body.h"
#include "base/error.h"
#include "base/macros.h"
#include "base/protocol.h"
#include "base/pstdint.h"
#include "base/settings_manager.h"

#include "server/id_manager.h"
#include "server/world_manager.h"

#include "server/bullet.h"
#include "server/dummy.h"
#include "server/player.h"
#include "server/wall.h"
#include "server/station.h"

namespace bm {

// XXX(xairy): load dynamic, category and mask from cfg?
Entity::Entity(
  WorldManager* world_manager,
  uint32_t id,
  const std::string& prefix,
  b2Vec2 position,
  uint16_t collision_category,
  uint16_t collision_mask
) : _world_manager(world_manager),
    _id(id),
    _is_destroyed(false),
    _is_updated(true) {
  SettingsManager* settings = world_manager->GetSettings();
  b2World* world = world_manager->GetWorld();
  body_ = new Body();
  CHECK(body_ != NULL);
  body_->Create(world, settings, prefix + ".shape");
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

WorldManager* Entity::GetWorldManager() {
  return _world_manager;
}

uint32_t Entity::GetId() const {
  return _id;
}

b2Vec2 Entity::GetPosition() const {
  return body_->GetPosition();
}
void Entity::SetPosition(const b2Vec2& position) {
  body_->SetPosition(position);
}

b2Vec2 Entity::GetVelocity() const {
  return body_->GetVelocity();
}

void Entity::SetVelocity(const b2Vec2& velocity) {
  body_->SetVelocity(velocity);
}

void Entity::Destroy() {
  _is_destroyed = true;
}
bool Entity::IsDestroyed() const {
  return _is_destroyed;
}

void Entity::SetUpdatedFlag(bool value) {
  _is_updated = value;
}
bool Entity::IsUpdated() const {
  return _is_updated;
}

// Double dispatch.

void Entity::Collide(Station* first, Station* second) {
  first->GetWorldManager()->OnCollision(first, second);
}
void Entity::Collide(Station* first, Wall* second) {
  first->GetWorldManager()->OnCollision(first, second);
}
void Entity::Collide(Station* first, Player* second) {
  first->GetWorldManager()->OnCollision(first, second);
}
void Entity::Collide(Station* first, Dummy* second) {
  first->GetWorldManager()->OnCollision(first, second);
}
void Entity::Collide(Station* first, Bullet* second) {
  first->GetWorldManager()->OnCollision(first, second);
}

void Entity::Collide(Wall* first, Wall* second) {
  first->GetWorldManager()->OnCollision(first, second);
}
void Entity::Collide(Wall* first, Player* second) {
  first->GetWorldManager()->OnCollision(first, second);
}
void Entity::Collide(Wall* first, Dummy* second) {
  first->GetWorldManager()->OnCollision(first, second);
}
void Entity::Collide(Wall* first, Bullet* second) {
  first->GetWorldManager()->OnCollision(first, second);
}

void Entity::Collide(Player* first, Player* second) {
  first->GetWorldManager()->OnCollision(first, second);
}
void Entity::Collide(Player* first, Dummy* second) {
  first->GetWorldManager()->OnCollision(first, second);
}
void Entity::Collide(Player* first, Bullet* second) {
  first->GetWorldManager()->OnCollision(first, second);
}

void Entity::Collide(Dummy* first, Dummy* second) {
  first->GetWorldManager()->OnCollision(first, second);
}
void Entity::Collide(Dummy* first, Bullet* second) {
  first->GetWorldManager()->OnCollision(first, second);
}

void Entity::Collide(Bullet* first, Bullet* second) {
  first->GetWorldManager()->OnCollision(first, second);
}

}  // namespace bm
