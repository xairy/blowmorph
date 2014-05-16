// Copyright (c) 2013 Blowmorph Team

#ifndef SERVER_DUMMY_H_
#define SERVER_DUMMY_H_

#include <string>

#include "base/macros.h"
#include "base/protocol.h"
#include "base/pstdint.h"

#include "server/entity.h"
#include "server/vector.h"

namespace bm {

class Dummy : public Entity {
  friend class Entity;

 public:
  static Dummy* Create(
    WorldManager* world_manager,
    uint32_t id,
    const Vector2f& position,
    int64_t time);
  virtual ~Dummy();

  virtual std::string GetType();
  virtual bool IsStatic();

  virtual void Update(int64_t time);
  virtual void GetSnapshot(int64_t time, EntitySnapshot* output);

  virtual void OnEntityAppearance(Entity* entity);
  virtual void OnEntityDisappearance(Entity* entity);

  virtual void Damage(int damage, uint32_t id);

  virtual void SetPosition(const Vector2f& position);

  // Double dispatch. Collision detection.

  virtual bool Collide(Entity* entity);

  virtual bool Collide(Player* other);
  virtual bool Collide(Dummy* other);
  virtual bool Collide(Bullet* other);
  virtual bool Collide(Wall* other);
  virtual bool Collide(Station* other);

 protected:
  Dummy(WorldManager* world_manager, uint32_t source_id);

  float _speed;
  Entity* _meat;
  int64_t _last_update;
  Vector2f _prev_position;

 private:
  DISALLOW_COPY_AND_ASSIGN(Dummy);
};

}  // namespace bm

#endif  // SERVER_DUMMY_H_
