// Copyright (c) 2015 Blowmorph Team

#ifndef SERVER_PROJECTILE_H_
#define SERVER_PROJECTILE_H_

#include <string>

#include <Box2D/Box2D.h>

#include "base/macros.h"
#include "base/protocol.h"
#include "base/pstdint.h"

#include "server/entity.h"

namespace bm {

class Controller;

class Projectile : public Entity {
  friend class Entity;

 public:
  enum Type {
    TYPE_ROCKET,
    TYPE_SLIME,
  };

  Projectile(
    Controller* controller,
    uint32_t id,
    uint32_t owner_id,
    const b2Vec2& start,
    const b2Vec2& end,
    Type type);
  virtual ~Projectile();

  virtual Entity::Type GetType();
  virtual bool IsStatic();

  virtual void GetSnapshot(int64_t time, EntitySnapshot* output);

  virtual void Damage(int damage, uint32_t source_id);

  uint32_t GetOwnerId() const;
  Type GetProjectileType() const;

  // Double dispatch. Collision detection.

  virtual void Collide(Entity* entity);

  virtual void Collide(Player* other);
  virtual void Collide(Critter* other);
  virtual void Collide(Projectile* other);
  virtual void Collide(Wall* other);
  virtual void Collide(Kit* other);
  virtual void Collide(Activator* other);

 protected:
  uint32_t _owner_id;
  Type _type;

 private:
  DISALLOW_COPY_AND_ASSIGN(Projectile);
};

}  // namespace bm

#endif  // SERVER_PROJECTILE_H_
