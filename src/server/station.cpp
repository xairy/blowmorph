// Copyright (c) 2013 Blowmorph Team

#include "server/station.h"

#include <memory>
#include <string>

#include <Box2D/Box2D.h>

#include "base/error.h"
#include "base/macros.h"
#include "base/protocol.h"
#include "base/pstdint.h"

#include "server/box2d_utils.h"
#include "server/world_manager.h"

namespace bm {

Station::Station(
  WorldManager* world_manager,
  uint32_t id,
  const b2Vec2& position,
  int health_regeneration,
  int blow_regeneration,
  int morph_regeneration,
  Type type
) : Entity(world_manager, id, "kit", position, false,
           Entity::FILTER_KIT, Entity::FILTER_ALL & ~Entity::FILTER_BULLET) {
  _health_regeneration = health_regeneration;
  _blow_regeneration = blow_regeneration;
  _morph_regeneration = morph_regeneration;
  _type = type;
}

Station::~Station() { }

Entity::Type Station::GetType() {
  return Entity::TYPE_KIT;
}
bool Station::IsStatic() {
  return false;
}

void Station::GetSnapshot(int64_t time, EntitySnapshot* output) {
  output->type = EntitySnapshot::ENTITY_TYPE_STATION;
  output->time = time;
  output->id = _id;
  output->x = body_->GetPosition().x;
  output->y = body_->GetPosition().y;
  if (_type == TYPE_HEALTH) {
    output->data[0] = EntitySnapshot::STATION_TYPE_HEALTH;
  } else if (_type == TYPE_BLOW) {
    output->data[0] = EntitySnapshot::STATION_TYPE_BLOW;
  } else if (_type == TYPE_MORPH) {
    output->data[0] = EntitySnapshot::STATION_TYPE_MORPH;
  } else if (_type == TYPE_COMPOSITE) {
    output->data[0] = EntitySnapshot::STATION_TYPE_COMPOSITE;
  } else {
    CHECK(false);
  }
}

void Station::Damage(int damage, uint32_t source_id) { }

int Station::GetHealthRegeneration() const {
  return _health_regeneration;
}
int Station::GetBlowRegeneration() const {
  return _blow_regeneration;
}
int Station::GetMorphRegeneration() const {
  return _morph_regeneration;
}

// Double dispatch. Collision detection.

void Station::Collide(Entity* entity) {
  entity->Collide(this);
}

void Station::Collide(Player* other) {
  Entity::Collide(this, other);
}
void Station::Collide(Dummy* other) {
  Entity::Collide(this, other);
}
void Station::Collide(Bullet* other) {
  Entity::Collide(this, other);
}
void Station::Collide(Wall* other) {
  Entity::Collide(this, other);
}
void Station::Collide(Station* other) {
  Entity::Collide(other, this);
}

}  // namespace bm
