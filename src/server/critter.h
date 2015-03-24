// Copyright (c) 2015 Blowmorph Team

#ifndef SERVER_CRITTER_H_
#define SERVER_CRITTER_H_

#include <string>

#include <Box2D/Box2D.h>

#include "base/macros.h"
#include "base/pstdint.h"

#include "engine/protocol.h"

#include "server/entity.h"

namespace bm {

class Controller;

class Critter : public ServerEntity {
  friend class ServerEntity;

 public:
  enum Type {
    TYPE_ZOMBIE
  };

 public:
  Critter(
    Controller* controller,
    uint32_t id,
    const b2Vec2& position,
    const std::string& entity_name);
  virtual ~Critter();

  virtual void GetSnapshot(int64_t time, EntitySnapshot* output);
  virtual void Damage(int damage, uint32_t id);

  float GetSpeed() const;

  Entity* GetTarget() const;
  void SetTarget(Entity* target);

  // Double dispatch. Collision detection.
  virtual void Collide(ServerEntity* entity);
  virtual void Collide(Activator* other);
  virtual void Collide(Critter* other);
  virtual void Collide(Door* other);
  virtual void Collide(Kit* other);
  virtual void Collide(Player* other);
  virtual void Collide(Projectile* other);
  virtual void Collide(Wall* other);

 protected:
  float _speed;
  Entity* _target;
  Type type_;

 private:
  DISALLOW_COPY_AND_ASSIGN(Critter);
};

}  // namespace bm

#endif  // SERVER_CRITTER_H_
