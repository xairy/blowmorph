// Copyright (c) 2013 Blowmorph Team

#ifndef SERVER_BULLET_H_
#define SERVER_BULLET_H_

#include <string>

#include <Box2D/Box2D.h>

#include "base/macros.h"
#include "base/protocol.h"
#include "base/pstdint.h"

#include "server/entity.h"

namespace bm {

// FIXME(xairy): bullet's speed seems to be limited.
class Bullet : public Entity {
  friend class Entity;

 public:
  Bullet(
    WorldManager* world_manager,
    uint32_t id,
    uint32_t owner_id,
    const b2Vec2& start,
    const b2Vec2& end);
  virtual ~Bullet();

  virtual Entity::Type GetType();
  virtual bool IsStatic();

  virtual void GetSnapshot(int64_t time, EntitySnapshot* output);

  virtual void Damage(int damage, uint32_t source_id);

  uint32_t GetOwnerId();

  // Double dispatch. Collision detection.

  virtual void Collide(Entity* entity);

  virtual void Collide(Player* other);
  virtual void Collide(Dummy* other);
  virtual void Collide(Bullet* other);
  virtual void Collide(Wall* other);
  virtual void Collide(Kit* other);

 protected:
  uint32_t _owner_id;

 private:
  DISALLOW_COPY_AND_ASSIGN(Bullet);
};

}  // namespace bm

#endif  // SERVER_BULLET_H_
