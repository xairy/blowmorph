// Copyright (c) 2015 Blowmorph Team

#ifndef SERVER_ENTITY_H_
#define SERVER_ENTITY_H_

#include <string>

#include <Box2D/Box2D.h>

#include "base/id_manager.h"
#include "base/macros.h"
#include "base/pstdint.h"

#include "engine/body.h"
#include "engine/entity.h"
#include "engine/protocol.h"

namespace bm {

class Controller;

class Activator;
class Critter;
class Door;
class Kit;
class Player;
class Projectile;
class Wall;

class ServerEntity : public Entity {
 public:
  static const uint32_t BAD_ID = IdManager::BAD_ID;

 public:
  ServerEntity(
    Controller* controller,
    uint32_t id,
    Type type,
    const std::string& entity_name,
    b2Vec2 position,
    uint16_t collision_category,
    uint16_t collision_mask);
  virtual ~ServerEntity();

  Controller* GetController();

  // FIXME(xairy): get rid of it.
  void SetUpdatedFlag(bool value);
  bool IsUpdated() const;

  void Destroy();
  bool IsDestroyed() const;

  virtual void GetSnapshot(int64_t time, EntitySnapshot* output);
  virtual void Damage(int damage, uint32_t source_id);

  // Double dispatch. Collision handling.

  virtual void Collide(ServerEntity* entity) = 0;

  virtual void Collide(Player* other) = 0;
  virtual void Collide(Critter* other) = 0;
  virtual void Collide(Projectile* other) = 0;
  virtual void Collide(Wall* other) = 0;
  virtual void Collide(Kit* other) = 0;
  virtual void Collide(Activator* other) = 0;

  static void Collide(Door* first, Door* second);
  static void Collide(Door* first, Activator* second);
  static void Collide(Door* first, Kit* second);
  static void Collide(Door* first, Wall* second);
  static void Collide(Door* first, Player* second);
  static void Collide(Door* first, Critter* second);
  static void Collide(Door* first, Projectile* second);

  static void Collide(Activator* first, Activator* second);
  static void Collide(Activator* first, Kit* second);
  static void Collide(Activator* first, Wall* second);
  static void Collide(Activator* first, Player* second);
  static void Collide(Activator* first, Critter* second);
  static void Collide(Activator* first, Projectile* second);

  static void Collide(Kit* first, Kit* second);
  static void Collide(Kit* first, Wall* second);
  static void Collide(Kit* first, Player* second);
  static void Collide(Kit* first, Critter* second);
  static void Collide(Kit* first, Projectile* second);

  static void Collide(Wall* first, Wall* second);
  static void Collide(Wall* first, Player* second);
  static void Collide(Wall* first, Critter* second);
  static void Collide(Wall* first, Projectile* second);

  static void Collide(Player* first, Player* second);
  static void Collide(Player* first, Critter* second);
  static void Collide(Player* first, Projectile* second);

  static void Collide(Critter* first, Critter* second);
  static void Collide(Critter* first, Projectile* second);

  static void Collide(Projectile* first, Projectile* second);

 protected:
  Controller* controller_;

  bool is_destroyed_;
  bool is_updated_;
};

}  // namespace bm

#endif  // SERVER_ENTITY_H_
