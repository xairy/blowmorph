// Copyright (c) 2015 Blowmorph Team

#include "server/kit.h"

#include <memory>
#include <string>

#include <Box2D/Box2D.h>

#include "base/error.h"
#include "base/macros.h"
#include "base/pstdint.h"

#include "engine/body.h"
#include "engine/config.h"
#include "engine/protocol.h"

#include "server/controller.h"
#include "server/entity.h"

namespace bm {

Kit::Kit(
  Controller* controller,
  uint32_t id,
  const b2Vec2& position,
  const std::string& entity_name
) : ServerEntity(controller, id, Entity::TYPE_KIT, entity_name, position,
        Entity::FILTER_KIT, Entity::FILTER_ALL & ~Entity::FILTER_PROJECTILE) {
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
  _health_regeneration = config.at(entity_name).health_regen;
  _energy_regeneration = config.at(entity_name).energy_regen;
}

Kit::~Kit() { }

void Kit::GetSnapshot(int64_t time, EntitySnapshot* output) {
  output->type = EntitySnapshot::ENTITY_TYPE_KIT;
  output->time = time;
  output->id = GetId();
  output->x = GetPosition().x;
  output->y = GetPosition().y;
  output->angle = GetRotation();
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

void Kit::Collide(ServerEntity* entity) {
  entity->Collide(this);
}

void Kit::Collide(Player* other) {
  ServerEntity::Collide(this, other);
}
void Kit::Collide(Critter* other) {
  ServerEntity::Collide(this, other);
}
void Kit::Collide(Projectile* other) {
  ServerEntity::Collide(this, other);
}
void Kit::Collide(Wall* other) {
  ServerEntity::Collide(this, other);
}
void Kit::Collide(Kit* other) {
  ServerEntity::Collide(other, this);
}
void Kit::Collide(Activator* other) {
  ServerEntity::Collide(other, this);
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
