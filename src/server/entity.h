// Copyright (c) 2013 Blowmorph Team

#ifndef SERVER_ENTITY_H_
#define SERVER_ENTITY_H_

#include <string>

#include "base/macros.h"
#include "base/protocol.h"
#include "base/pstdint.h"

#include "server/vector.h"
#include "server/shape.h"

namespace bm {

class WorldManager;

class Player;
class Dummy;
class Bullet;
class Wall;
class Station;

class Entity {
 public:
  Entity(WorldManager* world_manager, uint32_t id);
  virtual ~Entity();

  // XXX(xairy): rename it to GetEntityType?
  virtual std::string GetType() = 0;
  virtual bool IsStatic() = 0;

  virtual void Update(int64_t time) = 0;
  virtual void GetSnapshot(int64_t time, EntitySnapshot* output) = 0;

  virtual void OnEntityAppearance(Entity* entity) = 0;
  virtual void OnEntityDisappearance(Entity* entity) = 0;

  virtual void Damage(int damage, uint32_t source_id) = 0;

  virtual uint32_t GetId() const;

  virtual Shape* GetShape();
  virtual void SetShape(Shape* shape);

  virtual Vector2f GetPosition() const;
  virtual void SetPosition(const Vector2f& position);

  virtual void Destroy();
  virtual bool IsDestroyed() const;

  virtual void SetUpdatedFlag(bool value);
  virtual bool IsUpdated() const;

  // Double dispatch. Collision detection.

  virtual bool Collide(Entity* entity) = 0;

  virtual bool Collide(Player* other) = 0;
  virtual bool Collide(Dummy* other) = 0;
  virtual bool Collide(Bullet* other) = 0;
  virtual bool Collide(Wall* other) = 0;
  virtual bool Collide(Station* other) = 0;

  static bool Collide(Station* station1, Station* station2);
  static bool Collide(Station* station, Wall* wall);
  static bool Collide(Station* station, Player* player);
  static bool Collide(Station* station, Dummy* dummy);
  static bool Collide(Station* station, Bullet* bullet);

  static bool Collide(Wall* wall1, Wall* wall2);
  static bool Collide(Wall* wall, Player* player);
  static bool Collide(Wall* wall, Dummy* dummy);
  static bool Collide(Wall* wall, Bullet* bullet);

  static bool Collide(Player* player1, Player* player2);
  static bool Collide(Player* player, Dummy* dummy);
  static bool Collide(Player* player, Bullet* bullet);

  static bool Collide(Dummy* dummy1, Dummy* dummy2);
  static bool Collide(Dummy* dummy, Bullet* bullet);

  static bool Collide(Bullet* bullet1, Bullet* bullet2);

 protected:
  WorldManager* _world_manager;

  uint32_t _id;
  Shape* _shape;

  bool _is_destroyed;
  bool _is_updated;
};

}  // namespace bm

#endif  // SERVER_ENTITY_H_
