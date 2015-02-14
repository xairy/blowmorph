// Copyright (c) 2015 Blowmorph Team

#ifndef SERVER_CONTROLLER_H_
#define SERVER_CONTROLLER_H_

#include <map>
#include <string>
#include <vector>

#include <Box2D/Box2D.h>

#include "base/protocol.h"
#include "base/pstdint.h"
#include "base/settings_manager.h"

#include "server/contact_listener.h"
#include "server/entity.h"
#include "server/world.h"

#include "server/activator.h"
#include "server/projectile.h"
#include "server/critter.h"
#include "server/kit.h"
#include "server/player.h"
#include "server/wall.h"

namespace bm {

class Controller {
 public:
  explicit Controller(IdManager* id_manager);
  ~Controller();

  World* GetWorld();
  SettingsManager* GetEntitySettings();

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

  void OnCollision(Activator* activator1, Activator* activator2);
  void OnCollision(Activator* activator, Kit* kit);
  void OnCollision(Activator* activator, Wall* wall);
  void OnCollision(Activator* activator, Player* player);
  void OnCollision(Activator* activator, Critter* critter);
  void OnCollision(Activator* activator, Projectile* projectile);

  void OnCollision(Kit* kit1, Kit* kit2);
  void OnCollision(Kit* kit, Wall* wall);
  void OnCollision(Kit* kit, Player* player);
  void OnCollision(Kit* kit, Critter* critter);
  void OnCollision(Kit* kit, Projectile* projectile);

  void OnCollision(Wall* wall1, Wall* wall2);
  void OnCollision(Wall* wall, Player* player);
  void OnCollision(Wall* wall, Critter* critter);
  void OnCollision(Wall* wall, Projectile* projectile);

  void OnCollision(Player* player1, Player* player2);
  void OnCollision(Player* player, Critter* critter);
  void OnCollision(Player* player, Projectile* projectile);

  void OnCollision(Critter* critter1, Critter* critter2);
  void OnCollision(Critter* critter, Projectile* projectile);

  void OnCollision(Projectile* projectile1, Projectile* projectile2);

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

  // Explosions.

  void ExplodeProjectile(Projectile* projectile);
  void ExplodeCritter(Critter* critter);
  void MakeExplosion(const b2Vec2& location, uint32_t source_id);
  void MakeSlimeExplosion(const b2Vec2& location);

  World world_;
  ContactListener contact_listener_;

  // TODO(xairy): refactor.
  std::vector<b2Vec2> morph_list_;

  std::vector<GameEvent> game_events_;

  SettingsManager entity_settings_;
  SettingsManager gun_settings_;
};

}  // namespace bm

#endif  // SERVER_CONTROLLER_H_
