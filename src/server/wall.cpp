// Copyright (c) 2013 Blowmorph Team

#include "server/wall.h"

#include <memory>
#include <string>

#include <Box2D/Box2D.h>

#include "base/error.h"
#include "base/macros.h"
#include "base/protocol.h"
#include "base/pstdint.h"
#include "base/settings_manager.h"

#include "server/box2d_utils.h"
#include "server/world_manager.h"

namespace bm {

Wall* Wall::Create(
  WorldManager* world_manager,
  uint32_t id,
  const b2Vec2& position,
  Type type
) {
  SettingsManager* settings = world_manager->GetSettings();

  Wall* wall = new Wall(world_manager, id);
  CHECK(wall != NULL);

  b2World* world = world_manager->GetWorld();
  b2Body* body = CreateBody(world, settings, "wall.shape", false);
  SetBodyPosition(body, position);
  body->SetUserData(wall);

  wall->body_ = body;
  wall->_type = type;

  return wall;
}

Wall::~Wall() { }

std::string Wall::GetType() {
  return "Wall";
}
bool Wall::IsStatic() {
  return true;
}

void Wall::Update(int64_t time) { }

void Wall::GetSnapshot(int64_t time, EntitySnapshot* output) {
  output->type = EntitySnapshot::ENTITY_TYPE_WALL;
  output->time = time;
  output->id = _id;
  output->x = body_->GetPosition().x;
  output->y = body_->GetPosition().y;
  if (_type == TYPE_ORDINARY) {
    output->data[0] = EntitySnapshot::WALL_TYPE_ORDINARY;
  } else if (_type == TYPE_UNBREAKABLE) {
    output->data[0] = EntitySnapshot::WALL_TYPE_UNBREAKABLE;
  } else if (_type == TYPE_MORPHED) {
    output->data[0] = EntitySnapshot::WALL_TYPE_MORPHED;
  } else {
    CHECK(false);
  }
}

void Wall::OnEntityAppearance(Entity* entity) { }
void Wall::OnEntityDisappearance(Entity* entity) { }

void Wall::Damage(int damage) {
  if (_type != TYPE_UNBREAKABLE) {
    Destroy();
  }
}

// Double dispatch. Collision detection.

void Wall::Collide(Entity* entity) {
  entity->Collide(this);
}

void Wall::Collide(Player* other) {
  Entity::Collide(this, other);
}
void Wall::Collide(Dummy* other) {
  Entity::Collide(this, other);
}
void Wall::Collide(Bullet* other) {
  Entity::Collide(this, other);
}
void Wall::Collide(Wall* other) {
  Entity::Collide(other, this);
}
void Wall::Collide(Station* other) {
  Entity::Collide(other, this);
}

Wall::Wall(WorldManager* world_manager, uint32_t id)
  : Entity(world_manager, id) { }

}  // namespace bm
