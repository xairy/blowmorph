// Copyright (c) 2015 Blowmorph Team

#ifndef SERVER_DUMMY_H_
#define SERVER_DUMMY_H_

#include <string>

#include <Box2D/Box2D.h>

#include "base/macros.h"
#include "base/protocol.h"
#include "base/pstdint.h"

#include "server/entity.h"

namespace bm {

class Controller;

// TODO(xairy): rename to Zombie.
class Dummy : public Entity {
  friend class Entity;

 public:
  Dummy(
    Controller* controller,
    uint32_t id,
    const b2Vec2& position);
  virtual ~Dummy();

  virtual Entity::Type GetType();
  virtual bool IsStatic();

  virtual void GetSnapshot(int64_t time, EntitySnapshot* output);

  virtual void Damage(int damage, uint32_t id);

  float GetSpeed() const;

  Entity* GetTarget() const;
  void SetTarget(Entity* target);

  // Double dispatch. Collision detection.

  virtual void Collide(Entity* entity);

  virtual void Collide(Player* other);
  virtual void Collide(Dummy* other);
  virtual void Collide(Bullet* other);
  virtual void Collide(Wall* other);
  virtual void Collide(Kit* other);
  virtual void Collide(Activator* other);

 protected:
  float _speed;
  Entity* _target;

 private:
  DISALLOW_COPY_AND_ASSIGN(Dummy);
};

}  // namespace bm

#endif  // SERVER_DUMMY_H_
