// Copyright (c) 2015 Blowmorph Team

#include "server/entity.h"

#include <cmath>

#include <memory>
#include <string>

#include <Box2D/Box2D.h>

#include "base/config_reader.h"
#include "base/error.h"
#include "base/macros.h"
#include "base/protocol.h"
#include "base/pstdint.h"
#include "engine/body.h"

#include "server/controller.h"
#include "server/id_manager.h"

#include "server/activator.h"
#include "server/projectile.h"
#include "server/critter.h"
#include "server/kit.h"
#include "server/player.h"
#include "server/wall.h"

namespace bm {

Entity::Entity(
  Controller* controller,
  uint32_t id,
  const std::string& body_config,
  b2Vec2 position,
  uint16_t collision_category,
  uint16_t collision_mask
) : controller_(controller),
    _id(id),
    _is_destroyed(false),
    _is_updated(true) {
  b2World* world = controller_->GetWorld()->GetBox2DWorld();
  body_ = new Body();
  CHECK(body_ != NULL);
  body_->Create(world, controller->GetBodySettings(), body_config);
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

Controller* Entity::GetController() {
  return controller_;
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

void Entity::Collide(Activator* first, Activator* second) {
  first->GetController()->OnCollision(first, second);
}
void Entity::Collide(Activator* first, Kit* second) {
  first->GetController()->OnCollision(first, second);
}
void Entity::Collide(Activator* first, Wall* second) {
  first->GetController()->OnCollision(first, second);
}
void Entity::Collide(Activator* first, Player* second) {
  first->GetController()->OnCollision(first, second);
}
void Entity::Collide(Activator* first, Critter* second) {
  first->GetController()->OnCollision(first, second);
}
void Entity::Collide(Activator* first, Projectile* second) {
  first->GetController()->OnCollision(first, second);
}

void Entity::Collide(Kit* first, Kit* second) {
  first->GetController()->OnCollision(first, second);
}
void Entity::Collide(Kit* first, Wall* second) {
  first->GetController()->OnCollision(first, second);
}
void Entity::Collide(Kit* first, Player* second) {
  first->GetController()->OnCollision(first, second);
}
void Entity::Collide(Kit* first, Critter* second) {
  first->GetController()->OnCollision(first, second);
}
void Entity::Collide(Kit* first, Projectile* second) {
  first->GetController()->OnCollision(first, second);
}

void Entity::Collide(Wall* first, Wall* second) {
  first->GetController()->OnCollision(first, second);
}
void Entity::Collide(Wall* first, Player* second) {
  first->GetController()->OnCollision(first, second);
}
void Entity::Collide(Wall* first, Critter* second) {
  first->GetController()->OnCollision(first, second);
}
void Entity::Collide(Wall* first, Projectile* second) {
  first->GetController()->OnCollision(first, second);
}

void Entity::Collide(Player* first, Player* second) {
  first->GetController()->OnCollision(first, second);
}
void Entity::Collide(Player* first, Critter* second) {
  first->GetController()->OnCollision(first, second);
}
void Entity::Collide(Player* first, Projectile* second) {
  first->GetController()->OnCollision(first, second);
}

void Entity::Collide(Critter* first, Critter* second) {
  first->GetController()->OnCollision(first, second);
}
void Entity::Collide(Critter* first, Projectile* second) {
  first->GetController()->OnCollision(first, second);
}

void Entity::Collide(Projectile* first, Projectile* second) {
  first->GetController()->OnCollision(first, second);
}

}  // namespace bm
