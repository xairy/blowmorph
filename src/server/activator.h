// Copyright (c) 2015 Blowmorph Team

#ifndef SERVER_ACTIVATOR_H_
#define SERVER_ACTIVATOR_H_

#include <string>

#include <Box2D/Box2D.h>

#include "base/macros.h"
#include "base/pstdint.h"

#include "engine/protocol.h"

#include "server/entity.h"

namespace bm {

class Controller;

class Activator : public ServerEntity {
  friend class ServerEntity;

 public:
  Activator(
    Controller* controller,
    uint32_t id,
    const b2Vec2& position,
    const std::string& entity_name);
  virtual ~Activator();

  // Inherited from Entity.
  virtual void GetSnapshot(int64_t time, EntitySnapshot* output);
  virtual void Damage(int damage, uint32_t source_id);

  float GetActivationDistance() const;

  void Activate(Entity* activator);

  // Double dispatch. Collision detection.
  virtual void Collide(ServerEntity* entity);
  virtual void Collide(Player* other);
  virtual void Collide(Critter* other);
  virtual void Collide(Projectile* other);
  virtual void Collide(Wall* other);
  virtual void Collide(Kit* other);
  virtual void Collide(Activator* other);

 private:
  float activation_distance_;

  DISALLOW_COPY_AND_ASSIGN(Activator);
};

}  // namespace bm

#endif  // SERVER_ACTIVATOR_H_
