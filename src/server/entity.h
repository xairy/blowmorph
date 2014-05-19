// Copyright (c) 2013 Blowmorph Team

#ifndef SERVER_ENTITY_H_
#define SERVER_ENTITY_H_

#include <string>

#include <Box2D/Box2D.h>

#include "base/macros.h"
#include "base/protocol.h"
#include "base/pstdint.h"

#include "server/id_manager.h"

namespace bm {

class WorldManager;

class Player;
class Dummy;
class Bullet;
class Wall;
class Station;

class Entity {
 public:
  static const uint32_t BAD_ID = IdManager::BAD_ID;

  // XXX(xairy): count Dummy as a Bullet?
  enum Type {
    TYPE_PLAYER,
    TYPE_BULLET,
    TYPE_WALL,
    TYPE_KIT,
    TYPE_DUMMY
  };

  // Collision filters.
  enum FilterType {
    FILTER_PLAYER = 0x0001,
    FILTER_BULLET = 0x0002,
    FILTER_WALL   = 0x0004,
    FILTER_KIT    = 0x0008,
    FILTER_ALL    = 0xffff,
    FILTER_NONE   = 0x0000
  };

 public:
  Entity(
    WorldManager* world_manager,
    uint32_t id,
    const std::string& prefix,
    b2Vec2 position,
    bool dynamic,
    uint16_t collision_category,
    uint16_t collision_mask);
  virtual ~Entity();

  WorldManager* GetWorldManager();

  virtual Type GetType() = 0;
  virtual bool IsStatic() = 0;

  virtual void GetSnapshot(int64_t time, EntitySnapshot* output) = 0;

  virtual void Damage(int damage, uint32_t source_id) = 0;

  virtual uint32_t GetId() const;

  virtual b2Vec2 GetPosition() const;
  virtual void SetPosition(const b2Vec2& position);

  virtual void SetVelocity(const b2Vec2& velocity);

  virtual void Destroy();
  virtual bool IsDestroyed() const;

  // FIXME(xairy): get rid of it.
  virtual void SetUpdatedFlag(bool value);
  virtual bool IsUpdated() const;

  // Double dispatch. Collision handling.

  virtual void Collide(Entity* entity) = 0;

  virtual void Collide(Player* other) = 0;
  virtual void Collide(Dummy* other) = 0;
  virtual void Collide(Bullet* other) = 0;
  virtual void Collide(Wall* other) = 0;
  virtual void Collide(Station* other) = 0;

  static void Collide(Station* station1, Station* station2);
  static void Collide(Station* station, Wall* wall);
  static void Collide(Station* station, Player* player);
  static void Collide(Station* station, Dummy* dummy);
  static void Collide(Station* station, Bullet* bullet);

  static void Collide(Wall* wall1, Wall* wall2);
  static void Collide(Wall* wall, Player* player);
  static void Collide(Wall* wall, Dummy* dummy);
  static void Collide(Wall* wall, Bullet* bullet);

  static void Collide(Player* player1, Player* player2);
  static void Collide(Player* player, Dummy* dummy);
  static void Collide(Player* player, Bullet* bullet);

  static void Collide(Dummy* dummy1, Dummy* dummy2);
  static void Collide(Dummy* dummy, Bullet* bullet);

  static void Collide(Bullet* bullet1, Bullet* bullet2);

 protected:
  WorldManager* _world_manager;

  uint32_t _id;

  bool _is_destroyed;
  bool _is_updated;

  b2Body* body_;
};

}  // namespace bm

#endif  // SERVER_ENTITY_H_
