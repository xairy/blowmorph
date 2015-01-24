// Copyright (c) 2013 Blowmorph Team

#ifndef SERVER_WORLD_MANAGER_H_
#define SERVER_WORLD_MANAGER_H_

#include <map>
#include <string>
#include <vector>

#include <pugixml.hpp>

#include <Box2D/Box2D.h>

#include "base/protocol.h"
#include "base/pstdint.h"
#include "base/settings_manager.h"

#include "server/contact_listener.h"
#include "server/entity.h"

#include "server/activator.h"
#include "server/bullet.h"
#include "server/dummy.h"
#include "server/kit.h"
#include "server/player.h"
#include "server/wall.h"

namespace bm {

class Entity;
class IdManager;

// FIXME(xairy): divide WorldManager into World and Controller.
class WorldManager {
 public:
  explicit WorldManager(IdManager* id_manager);
  ~WorldManager();

  b2World* GetWorld();
  SettingsManager* GetSettings();

  bool LoadMap(const std::string& file);

  void AddEntity(uint32_t id, Entity* entity);
  void DeleteEntity(uint32_t id, bool deallocate);
  void DeleteEntities(const std::vector<uint32_t>& input, bool deallocate);

  void OnEntityAppearance(Entity* entity);
  void OnEntityDisappearance(Entity* entity);

  Entity* GetEntity(uint32_t id);
  std::map<uint32_t, Entity*>* GetStaticEntities();
  std::map<uint32_t, Entity*>* GetDynamicEntities();
  void GetDestroyedEntities(std::vector<uint32_t>* output);

  // The list of the events should be cleared by the caller.
  std::vector<GameEvent>* GetGameEvents();

  void Update(int64_t time_delta);
  void UpdateEntities(int64_t time_delta);
  void StepPhysics(int64_t time_delta);
  void DestroyOutlyingEntities();

  void CreateBullet(
    uint32_t owner_id,
    const b2Vec2& start,
    const b2Vec2& end,
    Bullet::Type type);

  void CreateDummy(
    const b2Vec2& position);

  void CreateWall(
    const b2Vec2& position,
    Wall::Type type);

  void CreateKit(
    const b2Vec2& position,
    int health_regeneration,
    int energy_regeneration,
    Kit::Type type);

  void CreateActivator(
    const b2Vec2& position,
    Activator::Type type);

  // Works only with grid map.
  void CreateAlignedWall(float x, float y, Wall::Type type);

  // Works only with grid map.
  void MakeExplosion(const b2Vec2& location, uint32_t source_id);
  void MakeSlimeExplosion(const b2Vec2& location);

  void RespawnDeadPlayers();
  void RespawnPlayer(Player* player);
  void UpdateScore(Player* player);

  // Returns one of the spawn positions stored in '_spawn_positions'.
  b2Vec2 GetRandomSpawn() const;

  void OnKeyboardEvent(Player* player, const KeyboardEvent& event);
  void OnMouseEvent(Player* player, const MouseEvent& event);

  void OnPlayerAction(Player* player, const PlayerAction& event);

  void ExplodeBullet(Bullet* bullet);
  void ExplodeDummy(Dummy* dummy);

  // Collisions.

  void OnCollision(Activator* activator1, Activator* activator2);
  void OnCollision(Activator* activator, Kit* kit);
  void OnCollision(Activator* activator, Wall* wall);
  void OnCollision(Activator* activator, Player* player);
  void OnCollision(Activator* activator, Dummy* dummy);
  void OnCollision(Activator* activator, Bullet* bullet);

  void OnCollision(Kit* kit1, Kit* kit2);
  void OnCollision(Kit* kit, Wall* wall);
  void OnCollision(Kit* kit, Player* player);
  void OnCollision(Kit* kit, Dummy* dummy);
  void OnCollision(Kit* kit, Bullet* bullet);

  void OnCollision(Wall* wall1, Wall* wall2);
  void OnCollision(Wall* wall, Player* player);
  void OnCollision(Wall* wall, Dummy* dummy);
  void OnCollision(Wall* wall, Bullet* bullet);

  void OnCollision(Player* player1, Player* player2);
  void OnCollision(Player* player, Dummy* dummy);
  void OnCollision(Player* player, Bullet* bullet);

  void OnCollision(Dummy* dummy1, Dummy* dummy2);
  void OnCollision(Dummy* dummy, Bullet* bullet);

  void OnCollision(Bullet* bullet1, Bullet* bullet2);

 private:
  bool _LoadWall(const pugi::xml_node& node);
  bool _LoadChunk(const pugi::xml_node& node);
  bool _LoadSpawn(const pugi::xml_node& node);
  bool _LoadKit(const pugi::xml_node& node);

  bool _LoadWallType(const pugi::xml_attribute& attribute, Wall::Type* output);
  bool _LoadKitType(const pugi::xml_attribute& attr, Kit::Type* output);

  // Works only with grid map.
  void _CreateAlignedWall(int x, int y, Wall::Type type);

  b2World world_;
  ContactListener contact_listener_;

  std::map<uint32_t, Entity*> _static_entities;
  std::map<uint32_t, Entity*> _dynamic_entities;

  std::vector<b2Vec2> _spawn_positions;

  // TODO(xairy): refactor.
  std::vector<b2Vec2> _morph_list;

  std::vector<GameEvent> _game_events;

  // TODO(xairy): get rid of it.
  enum {
    MAP_NONE,
    MAP_GRID
  } _map_type;

  float _block_size;  // Works only with grid map.
  float _bound;  // Entities with greater coordinates are destroyed.

  IdManager* _id_manager;
  SettingsManager _settings;
};

}  // namespace bm

#endif  // SERVER_WORLD_MANAGER_H_
