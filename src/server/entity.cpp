// Copyright (c) 2013 Blowmorph Team

#include "server/entity.h"

#include <cmath>

#include <memory>
#include <string>

#include <Box2D/Box2D.h>

#include "base/error.h"
#include "base/macros.h"
#include "base/protocol.h"
#include "base/pstdint.h"
#include "base/settings_manager.h"

#include "server/box2d_utils.h"
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
  bool dynamic,
  uint16_t collision_category,
  uint16_t collision_mask
) : _world_manager(world_manager),
    _id(id),
    _is_destroyed(false),
    _is_updated(true) {
  SettingsManager* settings = world_manager->GetSettings();
  b2World* world = world_manager->GetWorld();
  body_ = CreateBody(world, settings, prefix + ".shape", dynamic);
  body_->SetUserData(this);
  SetBodyPosition(body_, position);
  SetCollisionFilter(body_, collision_category, collision_mask);
}

Entity::~Entity() {
  if (body_ != NULL) {
    body_->GetWorld()->DestroyBody(body_);
    body_ = NULL;
  }
}

uint32_t Entity::GetId() const {
  return _id;
}

b2Vec2 Entity::GetPosition() const {
  return body_->GetPosition();
}
void Entity::SetPosition(const b2Vec2& position) {
  SetBodyPosition(body_, position);
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

void Entity::Collide(Station* station1, Station* station2) { }
void Entity::Collide(Station* station, Wall* wall) { }

void Entity::Collide(Station* station, Player* player) {
  player->AddHealth(station->_health_regeneration);
  player->AddBlow(station->_blow_regeneration);
  player->AddMorph(station->_morph_regeneration);
  station->Destroy();
}

void Entity::Collide(Station* station, Dummy* dummy) { }
void Entity::Collide(Station* station, Bullet* bullet) { }

void Entity::Collide(Wall* wall1, Wall* wall2) { }
void Entity::Collide(Wall* wall, Player* player) { }
void Entity::Collide(Wall* wall, Dummy* dummy) {
  dummy->Explode();
}

void Entity::Collide(Wall* wall, Bullet* bullet) {
  bullet->Explode();
  wall->Damage(0);
}

void Entity::Collide(Player* player1, Player* player2) { }

void Entity::Collide(Player* player, Dummy* dummy) {
  SettingsManager* settings = dummy->_world_manager->GetSettings();
  int damage = settings->GetInt32("dummy.damage");
  dummy->Explode();
}

void Entity::Collide(Player* player, Bullet* bullet) {
  if (bullet->_owner_id == player->GetId()) {
    return;
  }
  bullet->Explode();
}

void Entity::Collide(Dummy* dummy1, Dummy* dummy2) { }

void Entity::Collide(Dummy* dummy, Bullet* bullet) {
  bullet->Explode();
  dummy->Explode();
}

void Entity::Collide(Bullet* bullet1, Bullet* bullet2) {
  bullet1->Explode();
  bullet2->Explode();
}

}  // namespace bm
