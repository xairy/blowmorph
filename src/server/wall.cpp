// Copyright (c) 2013 Blowmorph Team

#include "server/wall.h"

#include <memory>
#include <string>

#include "base/error.h"
#include "base/macros.h"
#include "base/protocol.h"
#include "base/pstdint.h"
#include "base/settings_manager.h"

#include "server/vector.h"
#include "server/shape.h"
#include "server/world_manager.h"

namespace bm {

Wall* Wall::Create(
  WorldManager* world_manager,
  uint32_t id,
  const Vector2f& position,
  Type type
) {
  std::auto_ptr<Wall> wall(new Wall(world_manager, id));
  CHECK(wall.get() != NULL);

  std::auto_ptr<Shape> shape(world_manager->LoadShape("wall.shape"));
  if (shape.get() == NULL) {
    return NULL;
  }
  shape->SetPosition(position);

  wall->_shape = shape.release();
  wall->_type = type;

  return wall.release();
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
  output->x = _shape->GetPosition().x;
  output->y = _shape->GetPosition().y;
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

void Wall::OnEntityAppearance(Entity* entity) {
}
void Wall::OnEntityDisappearance(Entity* entity) {
}

void Wall::Damage(int damage) {
  if (_type != TYPE_UNBREAKABLE) {
    Destroy();
  }
}

// Double dispatch. Collision detection.

bool Wall::Collide(Entity* entity) {
  return entity->Collide(this);
}

bool Wall::Collide(Player* other) {
  return Entity::Collide(this, other);
}
bool Wall::Collide(Dummy* other) {
  return Entity::Collide(this, other);
}
bool Wall::Collide(Bullet* other) {
  return Entity::Collide(this, other);
}
bool Wall::Collide(Wall* other) {
  return Entity::Collide(other, this);
}
bool Wall::Collide(Station* other) {
  return Entity::Collide(other, this);
}

Wall::Wall(WorldManager* world_manager, uint32_t id)
  : Entity(world_manager, id) { }

}  // namespace bm
