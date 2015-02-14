// Copyright (c) 2015 Blowmorph Team

#ifndef SERVER_ACTIVATOR_H_
#define SERVER_ACTIVATOR_H_

#include <string>

#include <Box2D/Box2D.h>

#include "base/macros.h"
#include "base/protocol.h"
#include "base/pstdint.h"

#include "server/entity.h"

namespace bm {

class Controller;

class Activator : public Entity {
  friend class Entity;

 public:
  enum Type {
    TYPE_DOOR
  };

 public:
  Activator(
    Controller* controller,
    uint32_t id,
    const b2Vec2& position,
    Type type);
  virtual ~Activator();

  // Inherited from Entity.
  virtual Entity::Type GetType();
  virtual bool IsStatic();
  virtual void GetSnapshot(int64_t time, EntitySnapshot* output);
  virtual void Damage(int damage, uint32_t source_id);

  void Activate(Entity* activator);

  // Double dispatch. Collision detection.
  virtual void Collide(Entity* entity);
  virtual void Collide(Player* other);
  virtual void Collide(Critter* other);
  virtual void Collide(Projectile* other);
  virtual void Collide(Wall* other);
  virtual void Collide(Kit* other);
  virtual void Collide(Activator* other);

 protected:
  Type type_;

 private:
  DISALLOW_COPY_AND_ASSIGN(Activator);
};

}  // namespace bm

#endif  // SERVER_ACTIVATOR_H_
