// Copyright (c) 2013 Blowmorph Team

#ifndef SERVER_DUMMY_H_
#define SERVER_DUMMY_H_

#include <string>

#include <Box2D/Box2D.h>

#include "base/macros.h"
#include "base/protocol.h"
#include "base/pstdint.h"

#include "server/entity.h"

namespace bm {

class Dummy : public Entity {
  friend class Entity;

 public:
  Dummy(
    WorldManager* world_manager,
    uint32_t id,
    const b2Vec2& position,
    int64_t time);
  virtual ~Dummy();

  virtual Entity::Type GetType();
  virtual bool IsStatic();

  virtual void Update(int64_t time);
  virtual void GetSnapshot(int64_t time, EntitySnapshot* output);

  virtual void OnEntityAppearance(Entity* entity);
  virtual void OnEntityDisappearance(Entity* entity);

  virtual void Damage(int damage);

  void Explode();

  // Double dispatch. Collision detection.

  virtual void Collide(Entity* entity);

  virtual void Collide(Player* other);
  virtual void Collide(Dummy* other);
  virtual void Collide(Bullet* other);
  virtual void Collide(Wall* other);
  virtual void Collide(Station* other);

 protected:
  float _speed;
  Entity* _meat;

 private:
  DISALLOW_COPY_AND_ASSIGN(Dummy);
};

}  // namespace bm

#endif  // SERVER_DUMMY_H_
