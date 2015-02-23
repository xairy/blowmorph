// Copyright (c) 2015 Blowmorph Team

#include "server/projectile.h"

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

namespace bm {

Projectile::Projectile(
  Controller* controller,
  uint32_t id,
  uint32_t owner_id,
  const b2Vec2& start,
  const b2Vec2& end,
  const std::string& entity_name
) : ServerEntity(controller, id, Entity::TYPE_PROJECTILE, entity_name, start,
        Entity::FILTER_PROJECTILE, Entity::FILTER_ALL & ~Entity::FILTER_KIT) {
  auto config = Config::GetInstance()->GetProjectilesConfig();
  CHECK(config.count(entity_name) == 1);
  float speed = config.at(entity_name).speed;

  b2Vec2 velocity = end - start;
  velocity.Normalize();
  velocity *= speed;
  body_->ApplyImpulse(body_->GetMass() * velocity);

  float angle = atan2f(-velocity.x, velocity.y);
  SetRotation(angle);

  owner_id_ = owner_id;

  Config::ProjectileConfig::Type type = config.at(entity_name).type;
  if (type == Config::ProjectileConfig::TYPE_ROCKET) {
    type_ = TYPE_ROCKET;
    rocket_explosion_radius_ = config.at(entity_name).
        rocket_config.explosion_radius;
    rocket_explosion_damage_ = config.at(entity_name).
        rocket_config.explosion_damage;
  } else if (type == Config::ProjectileConfig::TYPE_SLIME) {
    type_ = TYPE_SLIME;
    slime_explosion_radius_ = config.at(entity_name).
        slime_config.explosion_radius;
  } else {
    CHECK(false);  // Unreachable.
  }
}

Projectile::~Projectile() { }

void Projectile::GetSnapshot(int64_t time, EntitySnapshot* output) {
  output->type = EntitySnapshot::ENTITY_TYPE_PROJECTILE;
  output->time = time;
  output->id = GetId();
  output->x = GetPosition().x;
  output->y = GetPosition().y;
  output->angle = GetRotation();
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

void Projectile::Collide(ServerEntity* entity) {
  entity->Collide(this);
}

void Projectile::Collide(Player* other) {
  ServerEntity::Collide(other, this);
}
void Projectile::Collide(Critter* other) {
  ServerEntity::Collide(other, this);
}
void Projectile::Collide(Projectile* other) {
  ServerEntity::Collide(this, other);
}
void Projectile::Collide(Wall* other) {
  ServerEntity::Collide(other, this);
}
void Projectile::Collide(Kit* other) {
  ServerEntity::Collide(other, this);
}
void Projectile::Collide(Activator* other) {
  ServerEntity::Collide(other, this);
}

}  // namespace bm
