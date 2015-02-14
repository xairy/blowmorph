// Copyright (c) 2015 Blowmorph Team

#ifndef SERVER_ENTITY_H_
#define SERVER_ENTITY_H_

#include <string>

#include <Box2D/Box2D.h>

#include "base/body.h"
#include "base/macros.h"
#include "base/protocol.h"
#include "base/pstdint.h"

#include "server/id_manager.h"

namespace bm {

class Controller;

class Player;
class Critter;
class Bullet;
class Wall;
class Kit;
class Activator;

class Entity {
 public:
  static const uint32_t BAD_ID = IdManager::BAD_ID;

  enum Type {
    TYPE_PLAYER,
    TYPE_BULLET,
    TYPE_WALL,
    TYPE_KIT,
    TYPE_CRITTER,
    TYPE_ACTIVATOR
  };

  // Collision filters.
  enum FilterType {
    FILTER_PLAYER    = 0x0001,
    FILTER_BULLET    = 0x0002,
    FILTER_WALL      = 0x0004,
    FILTER_KIT       = 0x0008,
    FILTER_ACTIVATOR = 0x0010,
    FILTER_ALL       = 0xffff,
    FILTER_NONE      = 0x0000
  };

 public:
  Entity(
    Controller* controller,
    uint32_t id,
    const std::string& prefix,
    b2Vec2 position,
    uint16_t collision_category,
    uint16_t collision_mask);
  virtual ~Entity();

  Controller* GetController();

  virtual Type GetType() = 0;
  virtual bool IsStatic() = 0;

  virtual void GetSnapshot(int64_t time, EntitySnapshot* output) = 0;

  virtual void Damage(int damage, uint32_t source_id) = 0;

  virtual uint32_t GetId() const;

  virtual b2Vec2 GetPosition() const;
  virtual void SetPosition(const b2Vec2& position);

  virtual float GetRotation() const;
  virtual void SetRotation(float angle);

  virtual b2Vec2 GetVelocity() const;
  virtual void SetVelocity(const b2Vec2& velocity);

  virtual float GetMass() const;
  virtual void ApplyImpulse(const b2Vec2& impulse);
  virtual void SetImpulse(const b2Vec2& impulse);

  virtual void Destroy();
  virtual bool IsDestroyed() const;

  // FIXME(xairy): get rid of it.
  virtual void SetUpdatedFlag(bool value);
  virtual bool IsUpdated() const;

  // Double dispatch. Collision handling.

  virtual void Collide(Entity* entity) = 0;

  virtual void Collide(Player* other) = 0;
  virtual void Collide(Critter* other) = 0;
  virtual void Collide(Bullet* other) = 0;
  virtual void Collide(Wall* other) = 0;
  virtual void Collide(Kit* other) = 0;
  virtual void Collide(Activator* other) = 0;

  static void Collide(Activator* first, Activator* second);
  static void Collide(Activator* first, Kit* second);
  static void Collide(Activator* first, Wall* second);
  static void Collide(Activator* first, Player* second);
  static void Collide(Activator* first, Critter* second);
  static void Collide(Activator* first, Bullet* second);

  static void Collide(Kit* first, Kit* second);
  static void Collide(Kit* first, Wall* second);
  static void Collide(Kit* first, Player* second);
  static void Collide(Kit* first, Critter* second);
  static void Collide(Kit* first, Bullet* second);

  static void Collide(Wall* first, Wall* second);
  static void Collide(Wall* first, Player* second);
  static void Collide(Wall* first, Critter* second);
  static void Collide(Wall* first, Bullet* second);

  static void Collide(Player* first, Player* second);
  static void Collide(Player* first, Critter* second);
  static void Collide(Player* first, Bullet* second);

  static void Collide(Critter* first, Critter* second);
  static void Collide(Critter* first, Bullet* second);

  static void Collide(Bullet* first, Bullet* second);

 protected:
  Controller* controller_;

  uint32_t _id;

  bool _is_destroyed;
  bool _is_updated;

  Body* body_;
};

}  // namespace bm

#endif  // SERVER_ENTITY_H_
