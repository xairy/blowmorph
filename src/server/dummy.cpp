#include "dummy.hpp"

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

Dummy* Dummy::Create(
  WorldManager* world_manager,
  uint32_t id,
  const Vector2& position,
  float radius,
  float speed,
  uint32_t time
) {
  std::auto_ptr<Dummy> dummy(new Dummy(world_manager, id));
  if(dummy.get() == NULL) {
    Error::Set(Error::TYPE_MEMORY);
    return NULL;
  }
  std::auto_ptr<Circle> shape(new Circle(position, radius));
  if(shape.get() == NULL) {
    Error::Set(Error::TYPE_MEMORY);
    return NULL;
  }

  dummy->_shape = shape.release();
  dummy->_speed = speed;
  dummy->_meat = NULL;
  dummy->_last_update = time;
  dummy->_prev_position = position;

  return dummy.release();
}

Dummy::~Dummy() { }

std::string Dummy::GetType() {
  return "Dummy";
}
bool Dummy::IsStatic() {
  return false;
}

void Dummy::Update(uint32_t time) {
  _prev_position = _shape->GetPosition();
  if(_meat != NULL) {
    bm::uint32_t dt = time - _last_update;
    Vector2 direction = _meat->GetPosition() - GetPosition();
    direction.Normalize();
    _shape->Move(direction * _speed * static_cast<float>(dt));
  }
  _last_update = time;
}
void Dummy::GetSnapshot(uint32_t time, EntitySnapshot* output) {
  output->type = BM_ENTITY_DUMMY;
  output->time = time;
  output->id = _id;
  output->x = _shape->GetPosition().x;
  output->y = _shape->GetPosition().y;
}

void Dummy::OnEntityAppearance(Entity* entity) {
  if(entity->GetType() == "Player") {
    if(_meat == NULL) {
      _meat = entity;
    } else {
      float current_distance = (_meat->GetPosition() - GetPosition()).Magnitude();
      float new_distance = (entity->GetPosition() - GetPosition()).Magnitude();
      if(new_distance < current_distance) {
        _meat = entity;
      }
    }
  }
}
void Dummy::OnEntityDisappearance(Entity* entity) {
  if(_meat == entity) {
    _meat = NULL;
  }
}

void Dummy::Damage() {
  Destroy();
}

void Dummy::SetPosition(const Vector2& position) {
  _prev_position = position;
  _shape->SetPosition(position);
}

bool Dummy::Collide(Entity* entity) {
  return entity->Collide(this);
}

bool Dummy::Collide(Player* other) {
  return Entity::Collide(other, this);
}
bool Dummy::Collide(Dummy* other) {
  return Entity::Collide(other, this);
}
bool Dummy::Collide(Bullet* other) {
  return Entity::Collide(this, other);
}
bool Dummy::Collide(Wall* other) {
  return Entity::Collide(other, this);
}

Dummy::Dummy(WorldManager* world_manager, uint32_t id) : Entity(world_manager, id) { }

} // namespace bm
