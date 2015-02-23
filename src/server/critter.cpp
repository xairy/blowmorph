// Copyright (c) 2015 Blowmorph Team

#include "server/critter.h"

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

Critter::Critter(
  Controller* controller,
  uint32_t id,
  const b2Vec2& position,
  const std::string& entity_name
) : ServerEntity(controller, id, Entity::TYPE_CRITTER, entity_name, position,
           Entity::FILTER_CRITTER, Entity::FILTER_ALL & ~Entity::FILTER_KIT) {
  auto config = Config::GetInstance()->GetCrittersConfig();
  CHECK(config.count(entity_name) == 1);
  _speed = config.at(entity_name).speed;
  _target = NULL;
  Config::CritterConfig::Type type = config.at(entity_name).type;
  if (type == Config::CritterConfig::TYPE_ZOMBIE) {
    type_ = TYPE_ZOMBIE;
  } else {
    CHECK(false);  // Unreachable.
  }
}

Critter::~Critter() { }

void Critter::GetSnapshot(int64_t time, EntitySnapshot* output) {
  output->type = EntitySnapshot::ENTITY_TYPE_CRITTER;
  output->time = time;
  output->id = GetId();
  output->x = GetPosition().x;
  output->y = GetPosition().y;
  output->angle = GetRotation();
}

void Critter::Damage(int damage, uint32_t source_id) {
  Destroy();
}

float Critter::GetSpeed() const {
  return _speed;
}

Entity* Critter::GetTarget() const {
  return _target;
}

void Critter::SetTarget(Entity* target) {
  _target = target;
}

void Critter::Collide(ServerEntity* entity) {
  entity->Collide(this);
}

void Critter::Collide(Player* other) {
  ServerEntity::Collide(other, this);
}
void Critter::Collide(Critter* other) {
  ServerEntity::Collide(other, this);
}
void Critter::Collide(Projectile* other) {
  ServerEntity::Collide(this, other);
}
void Critter::Collide(Wall* other) {
  ServerEntity::Collide(other, this);
}
void Critter::Collide(Kit* other) {
  ServerEntity::Collide(other, this);
}
void Critter::Collide(Activator* other) {
  ServerEntity::Collide(other, this);
}

}  // namespace bm
