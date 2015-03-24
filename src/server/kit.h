// Copyright (c) 2015 Blowmorph Team

#ifndef SERVER_KIT_H_
#define SERVER_KIT_H_

#include <string>

#include <Box2D/Box2D.h>

#include "base/macros.h"
#include "base/pstdint.h"

#include "engine/protocol.h"

#include "server/entity.h"

namespace bm {

class Controller;

class Kit : public ServerEntity {
  friend class ServerEntity;

 public:
  Kit(
    Controller* controller,
    uint32_t id,
    const b2Vec2& position,
    const std::string& entity_name);
  virtual ~Kit();

  virtual void GetSnapshot(int64_t time, EntitySnapshot* output);
  virtual void Damage(int damage, uint32_t source_id);

  int GetHealthRegeneration() const;
  int GetEnergyRegeneration() const;

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
  int _health_regeneration;
  int _energy_regeneration;

 private:
  DISALLOW_COPY_AND_ASSIGN(Kit);
};

}  // namespace bm

#endif  // SERVER_KIT_H_
