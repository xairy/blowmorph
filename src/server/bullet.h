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

class Bullet : public Entity {
  friend class Entity;

 public:
  static Bullet* Create(
    WorldManager* world_manager,
    uint32_t id,
    uint32_t owner_id,
    const b2Vec2& start,
    const b2Vec2& end,
    int64_t time);
  virtual ~Bullet();

  virtual std::string GetType();
  virtual bool IsStatic();

  virtual void Update(int64_t time);
  virtual void GetSnapshot(int64_t time, EntitySnapshot* output);

  virtual void OnEntityAppearance(Entity* entity);
  virtual void OnEntityDisappearance(Entity* entity);

  virtual void Damage(int damage);

  void Explode();

  // Double dispatch. Collision detection.

  virtual bool Collide(Entity* entity);

  virtual bool Collide(Player* other);
  virtual bool Collide(Dummy* other);
  virtual bool Collide(Bullet* other);
  virtual bool Collide(Wall* other);
  virtual bool Collide(Station* other);

 protected:
  Bullet(WorldManager* world_manager, uint32_t id);

  uint32_t _owner_id;

 private:
  DISALLOW_COPY_AND_ASSIGN(Bullet);
};

}  // namespace bm

#endif  // SERVER_BULLET_H_
