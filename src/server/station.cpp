// Copyright (c) 2013 Blowmorph Team

#include "server/station.h"

#include <memory>
#include <string>

#include <Box2D/Box2D.h>

#include "base/error.h"
#include "base/macros.h"
#include "base/protocol.h"
#include "base/pstdint.h"

#include "server/world_manager.h"

namespace bm {

Station* Station::Create(
  WorldManager* world_manager,
  uint32_t id,
  const b2Vec2& position,
  int health_regeneration,
  int blow_regeneration,
  int morph_regeneration,
  Type type
) {
  Station* station = new Station(world_manager, id);
  CHECK(station != NULL);

  // !FIXME: cfg.
  station->body_ = CreateBox(world_manager->GetWorld(), position, b2Vec2(15.0f, 15.0f), false, station);

  station->_health_regeneration = health_regeneration;
  station->_blow_regeneration = blow_regeneration;
  station->_morph_regeneration = morph_regeneration;
  station->_type = type;

  return station;
}

Station::~Station() { }

std::string Station::GetType() {
  return "Station";
}
bool Station::IsStatic() {
  return false;
}

void Station::Update(int64_t time) { }

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

void Station::OnEntityAppearance(Entity* entity) { }
void Station::OnEntityDisappearance(Entity* entity) { }

void Station::Damage(int damage) { }

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

Station::Station(WorldManager* world_manager, uint32_t id)
  : Entity(world_manager, id) { }

}  // namespace bm
