#include "wall.hpp"

#include <memory>
#include <string>

#include <base/error.hpp>
#include <base/macros.hpp>
#include <base/protocol.hpp>
#include <base/pstdint.hpp>

#include "vector.hpp"
#include "shape.hpp"
#include "world_manager.hpp"

namespace bm {

Wall* Wall::Create(
  WorldManager* world_manager,
  uint32_t id,
  const Vector2& position,
  float size,
  Type type
) {
  std::auto_ptr<Wall> wall(new Wall(world_manager, id));
  if(wall.get() == NULL) {
    Error::Set(Error::TYPE_MEMORY);
    return NULL;
  }
  std::auto_ptr<Shape> shape(new Square(position, size));
  if(wall.get() == NULL) {
    Error::Set(Error::TYPE_MEMORY);
    return NULL;
  }
  
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

void Wall::Update(uint32_t time) { }

void Wall::GetSnapshot(uint32_t time, EntitySnapshot* output) {
  output->type = EntitySnapshot::BM_ENTITY_WALL;
  output->time = time;
  output->id = _id;
  output->x = _shape->GetPosition().x;
  output->y = _shape->GetPosition().y;
  if(_type == TYPE_ORDINARY) {
    output->data[0] = EntitySnapshot::BM_WALL_ORDINARY;
  } else if(_type == TYPE_UNBREAKABLE) {
    output->data[0] = EntitySnapshot::BM_WALL_UNBREAKABLE;
  } else if(_type == TYPE_MORPHED) {
    output->data[0] = EntitySnapshot::BM_WALL_MORPHED;
  } else {
    CHECK(false);
  }
}

void Wall::OnEntityAppearance(Entity* entity) {

}
void Wall::OnEntityDisappearance(Entity* entity) {

}

void Wall::Damage() {
  if(_type != TYPE_UNBREAKABLE) {
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

Wall::Wall(WorldManager* world_manager, uint32_t id) : Entity(world_manager, id) { }


} // namespace bm
