// Copyright (c) 2013 Blowmorph Team

#ifndef SERVER_STATION_H_
#define SERVER_STATION_H_

#include <string>

#include <Box2D/Box2D.h>

#include "base/macros.h"
#include "base/protocol.h"
#include "base/pstdint.h"

#include "server/entity.h"

namespace bm {

class Station : public Entity {
  friend class Entity;

 public:
  enum Type {
    TYPE_HEALTH,
    TYPE_BLOW,
    TYPE_MORPH,
    TYPE_COMPOSITE
  };

  static Station* Create(
    WorldManager* world_manager,
    uint32_t id,
    const b2Vec2& position,
    int health_regeneration,
    int blow_regeneration,
    int morph_regeneration,
    Type type);
  virtual ~Station();

  virtual Entity::Type GetType();
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
  Station(WorldManager* world_manager, uint32_t id);

  int _health_regeneration;
  int _blow_regeneration;
  int _morph_regeneration;

  Type _type;

 private:
  DISALLOW_COPY_AND_ASSIGN(Station);
};

}  // namespace bm

#endif  // SERVER_STATION_H_
