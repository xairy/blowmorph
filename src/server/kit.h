// Copyright (c) 2015 Blowmorph Team

#ifndef SERVER_KIT_H_
#define SERVER_KIT_H_

#include <string>

#include <Box2D/Box2D.h>

#include "base/macros.h"
#include "base/protocol.h"
#include "base/pstdint.h"

#include "server/entity.h"

namespace bm {

class Kit : public Entity {
  friend class Entity;

 public:
  enum Type {
    TYPE_HEALTH,
    TYPE_ENERGY,
    TYPE_COMPOSITE
  };

  Kit(
    WorldManager* world_manager,
    uint32_t id,
    const b2Vec2& position,
    int health_regeneration,
    int energy_regeneration,
    Type type);
  virtual ~Kit();

  virtual Entity::Type GetType();
  virtual bool IsStatic();

  virtual void GetSnapshot(int64_t time, EntitySnapshot* output);

  virtual void Damage(int damage, uint32_t source_id);

  int GetHealthRegeneration() const;
  int GetEnergyRegeneration() const;

  // Double dispatch. Collision detection.

  virtual void Collide(Entity* entity);

  virtual void Collide(Player* other);
  virtual void Collide(Dummy* other);
  virtual void Collide(Bullet* other);
  virtual void Collide(Wall* other);
  virtual void Collide(Kit* other);

 protected:
  int _health_regeneration;
  int _energy_regeneration;

  Type _type;

 private:
  std::string TypeToEntityName(Type type);

  DISALLOW_COPY_AND_ASSIGN(Kit);
};

}  // namespace bm

#endif  // SERVER_KIT_H_
