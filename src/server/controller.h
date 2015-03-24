// Copyright (c) 2015 Blowmorph Team

#ifndef SERVER_CONTROLLER_H_
#define SERVER_CONTROLLER_H_

#include <map>
#include <string>
#include <vector>
#include <utility>

#include <Box2D/Box2D.h>

#include "base/pstdint.h"

#include "server/contact_listener.h"
#include "server/entity.h"
#include "server/world.h"

namespace bm {

class Activator;
class Critter;
class Door;
class Kit;
class Player;
class Projectile;
class Wall;

class Controller {
 public:
  explicit Controller();
  ~Controller();

  ServerWorld* GetWorld();

  // The list of the events should be cleared by the caller.
  std::vector<GameEvent>* GetGameEvents();

  void Update(int64_t time, int64_t time_delta);

  // Events.

  Player* OnPlayerConnected();
  void OnPlayerDisconnected(Player* player);

  void OnEntityAppearance(Entity* entity);
  void OnEntityDisappearance(Entity* entity);

  void OnKeyboardEvent(Player* player, const KeyboardEvent& event);
  void OnMouseEvent(Player* player, const MouseEvent& event);

  void OnPlayerAction(Player* player, const PlayerAction& event);

  // Collisions.

  void OnCollision(Door* first, Door* second);
  void OnCollision(Door* first, Activator* second);
  void OnCollision(Door* first, Kit* second);
  void OnCollision(Door* first, Wall* second);
  void OnCollision(Door* first, Player* second);
  void OnCollision(Door* first, Critter* second);
  void OnCollision(Door* first, Projectile* second);

  void OnCollision(Activator* first, Activator* second);
  void OnCollision(Activator* first, Kit* second);
  void OnCollision(Activator* first, Wall* second);
  void OnCollision(Activator* first, Player* second);
  void OnCollision(Activator* first, Critter* second);
  void OnCollision(Activator* first, Projectile* second);

  void OnCollision(Kit* first, Kit* second);
  void OnCollision(Kit* first, Wall* second);
  void OnCollision(Kit* first, Player* second);
  void OnCollision(Kit* first, Critter* second);
  void OnCollision(Kit* first, Projectile* second);

  void OnCollision(Wall* first, Wall* second);
  void OnCollision(Wall* first, Player* second);
  void OnCollision(Wall* first, Critter* second);
  void OnCollision(Wall* first, Projectile* second);

  void OnCollision(Player* first, Player* second);
  void OnCollision(Player* first, Critter* second);
  void OnCollision(Player* first, Projectile* second);

  void OnCollision(Critter* first, Critter* second);
  void OnCollision(Critter* first, Projectile* second);

  void OnCollision(Projectile* first, Projectile* second);

 private:
  // Updating.

  void SpawnZombies();
  void UpdateEntities(int64_t time_delta);
  void StepPhysics(int64_t time_delta);
  void DestroyOutlyingEntities();
  void RespawnDeadPlayers();
  void RespawnPlayer(Player* player);
  void UpdateScore(Player* player);
  void DeleteDestroyedEntities(int64_t time, int64_t time_delta);

  // Projectiles.

  void DestroyProjectile(Projectile* projectile);
  void MakeRocketExplosion(const b2Vec2& location, float radius,
                           int damage, uint32_t source_id);
  void MakeSlimeExplosion(const b2Vec2& location, int radius);

  ServerWorld world_;
  ContactListener contact_listener_;

  // TODO(xairy): refactor.
  std::vector<std::pair<b2Vec2, int> > morph_list_;

  std::vector<GameEvent> game_events_;
};

}  // namespace bm

#endif  // SERVER_CONTROLLER_H_
