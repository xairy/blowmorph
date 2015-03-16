// Copyright (c) 2015 Blowmorph Team

#include "server/entity.h"

#include <cmath>

#include <algorithm>
#include <memory>
#include <string>

#include <Box2D/Box2D.h>

#include "base/error.h"
#include "base/macros.h"
#include "base/pstdint.h"

#include "engine/body.h"
#include "engine/config.h"
#include "engine/entity.h"
#include "engine/protocol.h"

#include "server/controller.h"

#include "server/activator.h"
#include "server/projectile.h"
#include "server/critter.h"
#include "server/kit.h"
#include "server/player.h"
#include "server/wall.h"

namespace bm {

ServerEntity::ServerEntity(
  Controller* controller,
  uint32_t id,
  Type type,
  const std::string& entity_name,
  b2Vec2 position,
  uint16_t collision_category,
  uint16_t collision_mask
) : Entity(controller->GetWorld()->GetBox2DWorld(), id, type,
           entity_name, position, collision_category, collision_mask),
    controller_(controller),
    is_destroyed_(false),
    is_updated_(true) { }

ServerEntity::~ServerEntity() { }

Controller* ServerEntity::GetController() {
  return controller_;
}

void ServerEntity::SetUpdatedFlag(bool value) {
  is_updated_ = value;
}
bool ServerEntity::IsUpdated() const {
  return is_updated_;
}

void ServerEntity::Destroy() {
  is_destroyed_ = true;
}
bool ServerEntity::IsDestroyed() const {
  return is_destroyed_;
}

void ServerEntity::GetSnapshot(int64_t time, EntitySnapshot* output) {
  output->time = time;
  output->id = GetId();
  output->x = GetPosition().x;
  output->y = GetPosition().y;
  output->angle = GetRotation();

  CHECK(name_.size() <= EntitySnapshot::MAX_NAME_LENGTH);
  std::copy(name_.begin(), name_.end(), &output->name[0]);
  output->name[name_.size()] = '\0';
}

void ServerEntity::Damage(int damage, uint32_t source_id) { }

// Double dispatch.

void ServerEntity::Collide(Activator* first, Activator* second) {
  first->GetController()->OnCollision(first, second);
}
void ServerEntity::Collide(Activator* first, Kit* second) {
  first->GetController()->OnCollision(first, second);
}
void ServerEntity::Collide(Activator* first, Wall* second) {
  first->GetController()->OnCollision(first, second);
}
void ServerEntity::Collide(Activator* first, Player* second) {
  first->GetController()->OnCollision(first, second);
}
void ServerEntity::Collide(Activator* first, Critter* second) {
  first->GetController()->OnCollision(first, second);
}
void ServerEntity::Collide(Activator* first, Projectile* second) {
  first->GetController()->OnCollision(first, second);
}

void ServerEntity::Collide(Kit* first, Kit* second) {
  first->GetController()->OnCollision(first, second);
}
void ServerEntity::Collide(Kit* first, Wall* second) {
  first->GetController()->OnCollision(first, second);
}
void ServerEntity::Collide(Kit* first, Player* second) {
  first->GetController()->OnCollision(first, second);
}
void ServerEntity::Collide(Kit* first, Critter* second) {
  first->GetController()->OnCollision(first, second);
}
void ServerEntity::Collide(Kit* first, Projectile* second) {
  first->GetController()->OnCollision(first, second);
}

void ServerEntity::Collide(Wall* first, Wall* second) {
  first->GetController()->OnCollision(first, second);
}
void ServerEntity::Collide(Wall* first, Player* second) {
  first->GetController()->OnCollision(first, second);
}
void ServerEntity::Collide(Wall* first, Critter* second) {
  first->GetController()->OnCollision(first, second);
}
void ServerEntity::Collide(Wall* first, Projectile* second) {
  first->GetController()->OnCollision(first, second);
}

void ServerEntity::Collide(Player* first, Player* second) {
  first->GetController()->OnCollision(first, second);
}
void ServerEntity::Collide(Player* first, Critter* second) {
  first->GetController()->OnCollision(first, second);
}
void ServerEntity::Collide(Player* first, Projectile* second) {
  first->GetController()->OnCollision(first, second);
}

void ServerEntity::Collide(Critter* first, Critter* second) {
  first->GetController()->OnCollision(first, second);
}
void ServerEntity::Collide(Critter* first, Projectile* second) {
  first->GetController()->OnCollision(first, second);
}

void ServerEntity::Collide(Projectile* first, Projectile* second) {
  first->GetController()->OnCollision(first, second);
}

}  // namespace bm
