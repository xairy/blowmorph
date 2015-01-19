// Copyright (c) 2015 Blowmorph Team

#include "server/kit.h"

#include <memory>
#include <string>

#include <Box2D/Box2D.h>

#include "base/body.h"
#include "base/error.h"
#include "base/macros.h"
#include "base/protocol.h"
#include "base/pstdint.h"

#include "server/world_manager.h"

namespace bm {

Kit::Kit(
  WorldManager* world_manager,
  uint32_t id,
  const b2Vec2& position,
  int health_regeneration,
  int blow_regeneration,
  int morph_regeneration,
  Type type
) : Entity(world_manager, id, TypeToEntityName(type), position,
           Entity::FILTER_KIT, Entity::FILTER_ALL & ~Entity::FILTER_BULLET) {
  _health_regeneration = health_regeneration;
  _blow_regeneration = blow_regeneration;
  _morph_regeneration = morph_regeneration;
  _type = type;
}

Kit::~Kit() { }

Entity::Type Kit::GetType() {
  return Entity::TYPE_KIT;
}
bool Kit::IsStatic() {
  return false;
}

void Kit::GetSnapshot(int64_t time, EntitySnapshot* output) {
  output->type = EntitySnapshot::ENTITY_TYPE_KIT;
  output->time = time;
  output->id = _id;
  output->x = body_->GetPosition().x;
  output->y = body_->GetPosition().y;
  if (_type == TYPE_HEALTH) {
    output->data[0] = EntitySnapshot::KIT_TYPE_HEALTH;
  } else if (_type == TYPE_BLOW) {
    output->data[0] = EntitySnapshot::KIT_TYPE_BLOW;
  } else if (_type == TYPE_MORPH) {
    output->data[0] = EntitySnapshot::KIT_TYPE_MORPH;
  } else if (_type == TYPE_COMPOSITE) {
    output->data[0] = EntitySnapshot::KIT_TYPE_COMPOSITE;
  } else {
    CHECK(false);
  }
}

void Kit::Damage(int damage, uint32_t source_id) { }

int Kit::GetHealthRegeneration() const {
  return _health_regeneration;
}
int Kit::GetBlowRegeneration() const {
  return _blow_regeneration;
}
int Kit::GetMorphRegeneration() const {
  return _morph_regeneration;
}

// Double dispatch. Collision detection.

void Kit::Collide(Entity* entity) {
  entity->Collide(this);
}

void Kit::Collide(Player* other) {
  Entity::Collide(this, other);
}
void Kit::Collide(Dummy* other) {
  Entity::Collide(this, other);
}
void Kit::Collide(Bullet* other) {
  Entity::Collide(this, other);
}
void Kit::Collide(Wall* other) {
  Entity::Collide(this, other);
}
void Kit::Collide(Kit* other) {
  Entity::Collide(other, this);
}

std::string Kit::TypeToEntityName(Kit::Type type) {
  switch (type) {
    case Kit::TYPE_HEALTH:
      return "health_kit";
    case Kit::TYPE_BLOW:
      return "blow_kit";
    case Kit::TYPE_MORPH:
      return "morph_kit";
    case Kit::TYPE_COMPOSITE:
      return "composite_kit";
    default:
      CHECK(false);
  }
}

}  // namespace bm
