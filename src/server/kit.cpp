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

#include "server/controller.h"
#include "server/entity.h"

namespace bm {

Kit::Kit(
  Controller* controller,
  uint32_t id,
  const b2Vec2& position,
  int health_regeneration,
  int energy_regeneration,
  const std::string& config_name
) : Entity(controller, id,
           controller->GetKitSettings()->GetString(config_name + ".body"),
           position, Entity::FILTER_KIT,
           Entity::FILTER_ALL & ~Entity::FILTER_PROJECTILE) {
  _health_regeneration = health_regeneration;
  _energy_regeneration = energy_regeneration;
  SettingsManager* entity_settings = controller->GetKitSettings();
  std::string type_name = entity_settings->GetString(config_name + ".type");
  if (type_name == "health") {
    _type = TYPE_HEALTH;
  } else if (type_name == "energy") {
    _type = TYPE_ENERGY;
  } else if (type_name == "composite") {
    _type = TYPE_COMPOSITE;
  } else {
    CHECK(false);  // Unreachable.
  }
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
