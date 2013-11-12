// Copyright (c) 2013 Blowmorph Team

#include "server/dummy.h"

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

Dummy* Dummy::Create(
  WorldManager* world_manager,
  uint32_t id,
  const Vector2f& position,
  TimeType time
) {
  SettingsManager* settings = world_manager->GetSettings();
  float speed = settings->GetFloat("dummy.speed");

  std::auto_ptr<Dummy> dummy(new Dummy(world_manager, id));
  CHECK(dummy.get() != NULL);

  std::auto_ptr<Shape> shape(world_manager->LoadShape("dummy.shape"));
  if (shape.get() == NULL) {
    return NULL;
  }
  shape->SetPosition(position);

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

void Dummy::Update(TimeType time) {
  _prev_position = _shape->GetPosition();
  if (_meat != NULL) {
    TimeType dt = time - _last_update;
    Vector2f direction = _meat->GetPosition() - GetPosition();
    direction = Normalize(direction);
    _shape->Move(direction * _speed * static_cast<float>(dt));
  }
  _last_update = time;
}
void Dummy::GetSnapshot(TimeType time, EntitySnapshot* output) {
  output->type = EntitySnapshot::ENTITY_TYPE_DUMMY;
  output->time = time;
  output->id = _id;
  output->x = _shape->GetPosition().x;
  output->y = _shape->GetPosition().y;
}

void Dummy::OnEntityAppearance(Entity* entity) {
  if (entity->GetType() == "Player") {
    if (_meat == NULL) {
      _meat = entity;
    } else {
      float current_distance = Magnitude(_meat->GetPosition() - GetPosition());
      float new_distance = Magnitude(entity->GetPosition() - GetPosition());
      if (new_distance < current_distance) {
        _meat = entity;
      }
    }
  }
}
void Dummy::OnEntityDisappearance(Entity* entity) {
  if (_meat == entity) {
    _meat = NULL;
  }
}

void Dummy::Damage(int damage) {
  Destroy();
}

void Dummy::SetPosition(const Vector2f& position) {
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
bool Dummy::Collide(Station* other) {
  return Entity::Collide(other, this);
}

Dummy::Dummy(WorldManager* world_manager, uint32_t id)
  : Entity(world_manager, id) { }

}  // namespace bm
