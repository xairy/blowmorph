// Copyright (c) 2015 Blowmorph Team

#ifndef SERVER_WORLD_H_
#define SERVER_WORLD_H_

#include <map>
#include <vector>
#include <string>

#include <Box2D/Box2D.h>
#include <pugixml.hpp>

#include "base/pstdint.h"

#include "server/entity.h"
#include "server/id_manager.h"

#include "server/activator.h"
#include "server/projectile.h"
#include "server/critter.h"
#include "server/kit.h"
#include "server/player.h"
#include "server/wall.h"

namespace bm {

class Controller;

// Holds the current state of the world.
// Updated by the 'Controller' class.
class World {
 public:
  World(Controller* controller, IdManager* id_manager);
  ~World();

  b2World* GetBox2DWorld();
  float GetBound() const;
  float GetBlockSize() const;

  // TODO(xairy): use json as map file format.
  bool LoadMap(const std::string& file);

  Entity* GetEntity(uint32_t id);
  std::map<uint32_t, Entity*>* GetStaticEntities();
  std::map<uint32_t, Entity*>* GetDynamicEntities();

  // Doesn't delete the entity object.
  void RemoveEntity(uint32_t id);

  Activator* CreateActivator(
    const b2Vec2& position,
    const std::string& entity_name);

  Critter* CreateCritter(
    const b2Vec2& position,
    const std::string& entity_name);

  Kit* CreateKit(
    const b2Vec2& position,
    int health_regeneration,
    int energy_regeneration,
    const std::string& entity_name);

  Player* CreatePlayer(
    const b2Vec2& position,
    const std::string& entity_name);

  Projectile* CreateProjectile(
    uint32_t owner_id,
    const b2Vec2& start,
    const b2Vec2& end,
    const std::string& entity_name);

  Wall* CreateWall(
    const b2Vec2& position,
    const std::string& entity_name);

  std::vector<b2Vec2>* GetSpawnPositions();

 private:
  void AddEntity(uint32_t id, Entity* entity);

  bool LoadWall(const pugi::xml_node& node);
  bool LoadChunk(const pugi::xml_node& node);
  bool LoadSpawn(const pugi::xml_node& node);
  bool LoadKit(const pugi::xml_node& node);

  bool LoadKitType(const pugi::xml_attribute& attr, Kit::Type* output);

  b2World world_;

  float block_size_;
  float bound_;

  std::map<uint32_t, Entity*> static_entities_;
  std::map<uint32_t, Entity*> dynamic_entities_;

  std::vector<b2Vec2> spawn_positions_;

  IdManager* id_manager_;
  Controller* controller_;  // !refactor
};

}  // namespace bm

#endif  // SERVER_WORLD_H_
