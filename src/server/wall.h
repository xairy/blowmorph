// Copyright (c) 2015 Blowmorph Team

#ifndef SERVER_WALL_H_
#define SERVER_WALL_H_

#include <string>

#include <Box2D/Box2D.h>

#include "base/macros.h"
#include "base/pstdint.h"

#include "engine/protocol.h"

#include "server/entity.h"

namespace bm {

class Controller;

class Wall : public ServerEntity {
  friend class ServerEntity;

 public:
  enum Type {
    TYPE_ORDINARY,
    TYPE_UNBREAKABLE,
    TYPE_MORPHED
  };

  Wall(
    Controller* controller,
    uint32_t id,
    const b2Vec2& position,
    const std::string& entity_name);
  virtual ~Wall();

  virtual void GetSnapshot(int64_t time, EntitySnapshot* output);
  virtual void Damage(int damage, uint32_t source_id);

  // Double dispatch. Collision detection.
  virtual void Collide(ServerEntity* entity);
  virtual void Collide(Player* other);
  virtual void Collide(Critter* other);
  virtual void Collide(Projectile* other);
  virtual void Collide(Wall* other);
  virtual void Collide(Kit* other);
  virtual void Collide(Activator* other);

 private:
  Type _type;

  DISALLOW_COPY_AND_ASSIGN(Wall);
};

}  // namespace bm

#endif  // SERVER_WALL_H_
