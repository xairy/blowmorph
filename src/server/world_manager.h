// Copyright (c) 2013 Blowmorph Team

#ifndef SERVER_WORLD_MANAGER_H_
#define SERVER_WORLD_MANAGER_H_

#include <map>
#include <string>
#include <vector>

#include <pugixml.hpp>

#include <Box2D/Box2D.h>

#include "base/pstdint.h"
#include "base/settings_manager.h"

#include "server/contact_listener.h"
#include "server/entity.h"

#include "server/bullet.h"
#include "server/dummy.h"
#include "server/player.h"
#include "server/wall.h"
#include "server/station.h"

namespace bm {

class Entity;
class IdManager;

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

  Entity* GetEntity(uint32_t id);
  std::map<uint32_t, Entity*>* GetStaticEntities();
  std::map<uint32_t, Entity*>* GetDynamicEntities();
  void GetDestroyedEntities(std::vector<uint32_t>* output);

  void UpdateEntities(int64_t time);
  void StepPhysics(int64_t time_delta);
  void DestroyOutlyingEntities();

  void CreateBullet(
    uint32_t owner_id,
    const b2Vec2& start,
    const b2Vec2& end,
    int64_t time);

  void CreateDummy(
    const b2Vec2& position,
    int64_t time);

  void CreateWall(
    const b2Vec2& position,
    Wall::Type type);

  void CreateStation(
    const b2Vec2& position,
    int health_regeneration,
    int blow_regeneration,
    int morph_regeneration,
    Station::Type type);

  // Works only with grid map.
  void CreateAlignedWall(float x, float y, Wall::Type type);

  // Works only with grid map.
  void Blow(const b2Vec2& location, uint32_t source_id);
  void Morph(const b2Vec2& location);

  void RespawnDeadPlayers();
  void RespawnPlayer(Player* player);

  // Returns one of the spawn positions stored in '_spawn_positions'.
  b2Vec2 GetRandomSpawn() const;

 private:
  bool _LoadWall(const pugi::xml_node& node);
  bool _LoadChunk(const pugi::xml_node& node);
  bool _LoadSpawn(const pugi::xml_node& node);
  bool _LoadStation(const pugi::xml_node& node);

  bool _LoadWallType(const pugi::xml_attribute& attribute, Wall::Type* output);
  bool _LoadStationType(const pugi::xml_attribute& attr, Station::Type* output);

  // Works only with grid map.
  void _CreateAlignedWall(int x, int y, Wall::Type type);

  b2World world_;
  ContactListener contact_listener_;

  std::map<uint32_t, Entity*> _static_entities;
  std::map<uint32_t, Entity*> _dynamic_entities;

  std::vector<b2Vec2> _spawn_positions;

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
