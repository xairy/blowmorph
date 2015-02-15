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
) : Entity(controller, id,
           controller->GetProjectileSettings()->
              GetString(config_name + ".body"),
           start, Entity::FILTER_PROJECTILE,
           Entity::FILTER_ALL & ~Entity::FILTER_KIT) {
  SettingsManager* entity_settings = controller->GetProjectileSettings();
  float speed = entity_settings->GetFloat(config_name + ".speed");

  b2Vec2 velocity = end - start;
  velocity.Normalize();
  velocity *= speed;
  body_->ApplyImpulse(body_->GetMass() * velocity);

  owner_id_ = owner_id;

  std::string type_name = entity_settings->GetString(config_name + ".type");
  if (type_name == "rocket") {
    type_ = TYPE_ROCKET;
    rocket_explosion_radius_ = entity_settings->GetFloat(
        config_name + ".explosion_radius");
    rocket_explosion_damage_ = entity_settings->GetInt32(
        config_name + ".explosion_damage");
  } else if (type_name == "slime") {
    type_ = TYPE_SLIME;
    slime_explosion_radius_ = entity_settings->GetInt32(
        config_name + ".explosion_radius");
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
  if (type_ == TYPE_ROCKET) {
    output->data[0] = EntitySnapshot::PROJECTILE_TYPE_ROCKET;
  } else if (type_ == TYPE_SLIME) {
    output->data[0] = EntitySnapshot::PROJECTILE_TYPE_SLIME;
  } else {
    CHECK(false);
  }
}

void Projectile::Damage(int damage, uint32_t source_id) {
  Destroy();
}

uint32_t Projectile::GetOwnerId() const {
  return owner_id_;
}

Projectile::Type Projectile::GetProjectileType() const {
  return type_;
}

float Projectile::GetRocketExplosionRadius() const {
  return rocket_explosion_radius_;
}

int Projectile::GetRocketExplosionDamage() const {
  return rocket_explosion_damage_;
}

int Projectile::GetSlimeExplosionRadius() const {
  return slime_explosion_radius_;
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
