// Copyright (c) 2013 Blowmorph Team

#ifndef SERVER_WALL_H_
#define SERVER_WALL_H_

#include <string>

#include <Box2D/Box2D.h>

#include "base/macros.h"
#include "base/protocol.h"
#include "base/pstdint.h"

#include "server/entity.h"

namespace bm {

class Wall : public Entity {
  friend class Entity;

 public:
  enum Type {
    TYPE_ORDINARY,
    TYPE_UNBREAKABLE,
    TYPE_MORPHED
  };

  static Wall* Create(
    WorldManager* world_manager,
    uint32_t id,
    const b2Vec2& position,
    Type type);
  virtual ~Wall();

  virtual std::string GetType();
  virtual bool IsStatic();

  virtual void Update(int64_t time);
  virtual void GetSnapshot(int64_t time, EntitySnapshot* output);

  virtual void OnEntityAppearance(Entity* entity);
  virtual void OnEntityDisappearance(Entity* entity);

  virtual void Damage(int damage);

  // Double dispatch. Collision detection.

  virtual void Collide(Entity* entity);

  virtual void Collide(Player* other);
  virtual void Collide(Dummy* other);
  virtual void Collide(Bullet* other);
  virtual void Collide(Wall* other);
  virtual void Collide(Station* other);

 protected:
  Wall(WorldManager* world_manager, uint32_t id);

  Type _type;

 private:
  DISALLOW_COPY_AND_ASSIGN(Wall);
};

}  // namespace bm

#endif  // SERVER_WALL_H_
