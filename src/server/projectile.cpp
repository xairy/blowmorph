// Copyright (c) 2015 Blowmorph Team

#include "server/projectile.h"

#include <memory>
#include <string>

#include <Box2D/Box2D.h>

#include "base/body.h"
#include "base/error.h"
#include "base/macros.h"
#include "base/protocol.h"
#include "base/pstdint.h"
#include "base/settings_manager.h"

#include "server/controller.h"

namespace bm {

Projectile::Projectile(
  Controller* controller,
  uint32_t id,
  uint32_t owner_id,
  const b2Vec2& start,
  const b2Vec2& end,
  const std::string& config_name
) : Entity(controller, id, config_name, start, Entity::FILTER_PROJECTILE,
           Entity::FILTER_ALL & ~Entity::FILTER_KIT) {
  SettingsManager* entity_settings = controller->GetEntitySettings();
  float speed = entity_settings->GetFloat(config_name + ".speed");

  b2Vec2 velocity = end - start;
  velocity.Normalize();
  velocity *= speed;
  body_->ApplyImpulse(body_->GetMass() * velocity);

  _owner_id = owner_id;

  std::string type_name = entity_settings->GetString(config_name + ".type");
  if (type_name == "rocket") {
    _type = TYPE_ROCKET;
  } else if (type_name == "slime") {
    _type = TYPE_SLIME;
  } else {
    CHECK(false);  // Unreachable.
  }
}

Projectile::~Projectile() { }

Entity::Type Projectile::GetType() {
  return Entity::TYPE_PROJECTILE;
}
bool Projectile::IsStatic() {
  return false;
}

void Projectile::GetSnapshot(int64_t time, EntitySnapshot* output) {
  output->type = EntitySnapshot::ENTITY_TYPE_PROJECTILE;
  output->time = time;
  output->id = _id;
  output->x = body_->GetPosition().x;
  output->y = body_->GetPosition().y;
  output->angle = body_->GetRotation();
  if (_type == TYPE_ROCKET) {
    output->data[0] = EntitySnapshot::PROJECTILE_TYPE_ROCKET;
  } else if (_type == TYPE_SLIME) {
    output->data[0] = EntitySnapshot::PROJECTILE_TYPE_SLIME;
  } else {
    CHECK(false);
  }
}

void Projectile::Damage(int damage, uint32_t source_id) {
  Destroy();
}

uint32_t Projectile::GetOwnerId() const {
  return _owner_id;
}

Projectile::Type Projectile::GetProjectileType() const {
  return _type;
}

// Double dispatch. Collision detection.

void Projectile::Collide(Entity* entity) {
  entity->Collide(this);
}

void Projectile::Collide(Player* other) {
  Entity::Collide(other, this);
}
void Projectile::Collide(Critter* other) {
  Entity::Collide(other, this);
}
void Projectile::Collide(Projectile* other) {
  Entity::Collide(this, other);
}
void Projectile::Collide(Wall* other) {
  Entity::Collide(other, this);
}
void Projectile::Collide(Kit* other) {
  Entity::Collide(other, this);
}
void Projectile::Collide(Activator* other) {
  Entity::Collide(other, this);
}

}  // namespace bm
