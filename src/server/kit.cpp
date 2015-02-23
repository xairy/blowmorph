// Copyright (c) 2015 Blowmorph Team

#include "server/kit.h"

#include <memory>
#include <string>

#include <Box2D/Box2D.h>

#include "base/error.h"
#include "base/macros.h"
#include "base/protocol.h"
#include "base/pstdint.h"

#include "engine/body.h"
#include "engine/config.h"

#include "server/controller.h"
#include "server/entity.h"

namespace bm {

Kit::Kit(
  Controller* controller,
  uint32_t id,
  const b2Vec2& position,
  int health_regeneration,
  int energy_regeneration,
  const std::string& entity_name
) : Entity(controller, id, entity_name, Entity::TYPE_KIT, position,
          Entity::FILTER_KIT, Entity::FILTER_ALL & ~Entity::FILTER_PROJECTILE) {
  _health_regeneration = health_regeneration;
  _energy_regeneration = energy_regeneration;
  auto config = Config::GetInstance()->GetKitsConfig();
  CHECK(config.count(entity_name) == 1);
  Config::KitConfig::Type type = config.at(entity_name).type;
  if (type == Config::KitConfig::TYPE_HEALTH) {
    _type = TYPE_HEALTH;
  } else if (type == Config::KitConfig::TYPE_ENERGY) {
    _type = TYPE_ENERGY;
  } else if (type == Config::KitConfig::TYPE_COMPOSITE) {
    _type = TYPE_COMPOSITE;
  } else {
    CHECK(false);  // Unreachable.
  }
}

Kit::~Kit() { }

void Kit::GetSnapshot(int64_t time, EntitySnapshot* output) {
  output->type = EntitySnapshot::ENTITY_TYPE_KIT;
  output->time = time;
  output->id = _id;
  output->x = body_->GetPosition().x;
  output->y = body_->GetPosition().y;
  output->angle = body_->GetRotation();
  if (_type == TYPE_HEALTH) {
    output->data[0] = EntitySnapshot::KIT_TYPE_HEALTH;
  } else if (_type == TYPE_ENERGY) {
    output->data[0] = EntitySnapshot::KIT_TYPE_ENERGY;
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
int Kit::GetEnergyRegeneration() const {
  return _energy_regeneration;
}

// Double dispatch. Collision detection.

void Kit::Collide(Entity* entity) {
  entity->Collide(this);
}

void Kit::Collide(Player* other) {
  Entity::Collide(this, other);
}
void Kit::Collide(Critter* other) {
  Entity::Collide(this, other);
}
void Kit::Collide(Projectile* other) {
  Entity::Collide(this, other);
}
void Kit::Collide(Wall* other) {
  Entity::Collide(this, other);
}
void Kit::Collide(Kit* other) {
  Entity::Collide(other, this);
}
void Kit::Collide(Activator* other) {
  Entity::Collide(other, this);
}

std::string Kit::TypeToEntityName(Kit::Type type) {
  switch (type) {
    case Kit::TYPE_HEALTH:
      return "health_kit";
    case Kit::TYPE_ENERGY:
      return "energy_kit";
    case Kit::TYPE_COMPOSITE:
      return "composite_kit";
    default:
      CHECK(false);
  }
}

}  // namespace bm
