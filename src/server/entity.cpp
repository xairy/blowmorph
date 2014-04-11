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

#include "server/id_manager.h"
#include "server/world_manager.h"

#include "server/bullet.h"
#include "server/dummy.h"
#include "server/player.h"
#include "server/wall.h"
#include "server/station.h"

namespace bm {

Entity::Entity(WorldManager* world_manager, uint32_t id)
  : _world_manager(world_manager),
    _id(id),
    body_(NULL),
    _is_destroyed(false),
    _is_updated(true)
{ }
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
  body_->SetTransform(position, body_->GetAngle());
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
  player->RestoreHealth(station->_health_regeneration);
  player->RestoreBlow(station->_blow_regeneration);
  player->RestoreMorph(station->_morph_regeneration);
  station->Destroy();
}

void Entity::Collide(Station* station, Dummy* dummy) { }
void Entity::Collide(Station* station, Bullet* bullet) { }

void Entity::Collide(Wall* wall1, Wall* wall2) { }
void Entity::Collide(Wall* wall, Player* player) { }
void Entity::Collide(Wall* wall, Dummy* dummy) { }

void Entity::Collide(Wall* wall, Bullet* bullet) {
  bullet->Explode();
  wall->Damage(0);
}

void Entity::Collide(Player* player1, Player* player2) { }

void Entity::Collide(Player* player, Dummy* dummy) {
  SettingsManager* settings = dummy->_world_manager->GetSettings();
  int damage = settings->GetInt32("dummy.damage");
  player->Damage(damage);
  dummy->Damage(0);
}

void Entity::Collide(Player* player, Bullet* bullet) {
  if (bullet->_owner_id == player->GetId()) {
    return;
  }
  player->Damage(0);
  bullet->Explode();
}

void Entity::Collide(Dummy* dummy1, Dummy* dummy2) { }

void Entity::Collide(Dummy* dummy, Bullet* bullet) {
  bullet->Explode();
  dummy->Damage(0);
}

void Entity::Collide(Bullet* bullet1, Bullet* bullet2) {
  bullet1->Explode();
  bullet2->Explode();
}

}  // namespace bm
