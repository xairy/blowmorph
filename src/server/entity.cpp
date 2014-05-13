// Copyright (c) 2013 Blowmorph Team

#include "server/entity.h"

#include <cmath>

#include <memory>
#include <string>

#include "base/error.h"
#include "base/macros.h"
#include "base/protocol.h"
#include "base/pstdint.h"
#include "base/settings_manager.h"

#include "server/id_manager.h"
#include "server/vector.h"
#include "server/shape.h"
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
    _shape(NULL),
    _is_destroyed(false),
    _is_updated(true)
{ }
Entity::~Entity() {
  if (_shape != NULL) {
    delete _shape;
    _shape = NULL;
  }
}

uint32_t Entity::GetId() const {
  return _id;
}

Shape* Entity::GetShape() {
  return _shape;
}
void Entity::SetShape(Shape* shape) {
  _shape = shape;
}

Vector2f Entity::GetPosition() const {
  return _shape->GetPosition();
}
void Entity::SetPosition(const Vector2f& position) {
  _shape->SetPosition(position);
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

bool Entity::Collide(Station* station1, Station* station2) {
  return false;
}
bool Entity::Collide(Station* station, Wall* wall) {
  return false;
}
bool Entity::Collide(Station* station, Player* player) {
  if (station->_shape->Collide(player->_shape)) {
    player->RestoreHealth(station->_health_regeneration);
    player->RestoreBlow(station->_blow_regeneration);
    player->RestoreMorph(station->_morph_regeneration);
    station->Destroy();
    return true;
  }
  return false;
}
bool Entity::Collide(Station* station, Dummy* dummy) {
  return false;
}
bool Entity::Collide(Station* station, Bullet* bullet) {
  return false;
}
bool Entity::Collide(Wall* wall1, Wall* wall2) {
  return false;
}
bool Entity::Collide(Wall* wall, Player* player) {
  bool result = false;

  Vector2f px(player->_prev_position.x, player->_shape->GetPosition().y);
  Vector2f py(player->_shape->GetPosition().x, player->_prev_position.y);

  Vector2f position = player->_shape->GetPosition();

  player->_shape->SetPosition(px);
  if (player->_shape->Collide(wall->_shape)) {
    position.y = player->_prev_position.y;
    result &= true;
  }

  player->_shape->SetPosition(py);
  if (player->_shape->Collide(wall->_shape)) {
    position.x = player->_prev_position.x;
    result &= true;
  }

  player->_shape->SetPosition(position);

  return result;
}
bool Entity::Collide(Wall* wall, Dummy* dummy) {
  bool result = false;

  Vector2f px(dummy->_prev_position.x, dummy->_shape->GetPosition().y);
  Vector2f py(dummy->_shape->GetPosition().x, dummy->_prev_position.y);

  Vector2f position = dummy->_shape->GetPosition();

  dummy->_shape->SetPosition(px);
  if (dummy->_shape->Collide(wall->_shape)) {
    position.y = dummy->_prev_position.y;
    result &= true;
  }

  dummy->_shape->SetPosition(py);
  if (dummy->_shape->Collide(wall->_shape)) {
    position.x = dummy->_prev_position.x;
    result &= true;
  }

  dummy->_shape->SetPosition(position);

  return result;
}
bool Entity::Collide(Wall* wall, Bullet* bullet) {
  if (wall->_shape->Collide(bullet->_shape)) {
    bullet->Explode(bullet->_owner_id);
    // wall->Damage(0, owner_id);
    return true;
  }
  return false;
}
bool Entity::Collide(Player* player1, Player* player2) {
  return false;
}
bool Entity::Collide(Player* player, Dummy* dummy) {
  if (player->_shape->Collide(dummy->_shape)) {
    SettingsManager* settings = dummy->_world_manager->GetSettings();
    int damage = settings->GetInt32("dummy.damage");
    player->Damage(damage, dummy->_id);
    dummy->Damage(0, player->_id);
    return true;
  }
  return false;
}
bool Entity::Collide(Player* player, Bullet* bullet) {
  if (bullet->_owner_id == player->GetId()) {
    return false;
  }
  if (player->_shape->Collide(bullet->_shape)) {
    // player->Damage(0, owner_id);
    bullet->Explode(bullet->_owner_id);
    return true;
  }
  return false;
}
bool Entity::Collide(Dummy* dummy1, Dummy* dummy2) {
  return false;
}
bool Entity::Collide(Dummy* dummy, Bullet* bullet) {
  if (dummy->_shape->Collide(bullet->_shape)) {
    bullet->Explode(bullet->_owner_id);
    // dummy->Damage(0, owner_id);
    return true;
  }
  return false;
}
bool Entity::Collide(Bullet* bullet1, Bullet* bullet2) {
  if (bullet1->_shape->Collide(bullet2->_shape)) {
    bullet1->Explode(bullet1->_owner_id);
    bullet2->Explode(bullet1->_owner_id);
    return true;
  }
  return false;
}

}  // namespace bm
