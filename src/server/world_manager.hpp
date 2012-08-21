#ifndef BLOWMORPH_SERVER_WORLD_MANAGER_HPP_
#define BLOWMORPH_SERVER_WORLD_MANAGER_HPP_

#include <map>
#include <vector>

#include <pugixml.hpp>

#include <base/pstdint.hpp>

#include "entity.hpp"
#include "vector.hpp"

#include "bullet.hpp"
#include "dummy.hpp"
#include "player.hpp"
#include "wall.hpp"
#include "station.hpp"

namespace bm {

class Entity;
class IdManager;
class SettingsManager;

class WorldManager {
public:
  WorldManager(IdManager* id_manager, SettingsManager* settings);
  ~WorldManager();

  SettingsManager* GetSettings();

  bool LoadMap(const std::string& file);

  void AddEntity(uint32_t id, Entity* entity);
  void DeleteEntity(uint32_t id, bool deallocate);
  void DeleteEntities(const std::vector<uint32_t>& input, bool deallocate);

  Entity* GetEntity(uint32_t id);
  std::map<uint32_t, Entity*>* GetStaticEntities();
  std::map<uint32_t, Entity*>* GetDynamicEntities();
  void GetDestroyedEntities(std::vector<uint32_t>* output);
  
  void UpdateEntities(uint32_t time);
  void CollideEntities();
  void DestroyOutlyingEntities();

  bool CreateBullet(
    uint32_t owner_id,
    const Vector2f& start,
    const Vector2f& end,
    uint32_t time
  );
  bool CreateDummy(
    const Vector2f& position,
    uint32_t time
  );
  bool CreateWall(
    const Vector2f& position,
    Wall::Type type
  );
  bool CreateStation(
    const Vector2f& position,
    int health_regeneration,
    int blow_regeneration,
    int morph_regeneration,
    Station::Type type
  );

  // Works only with grid map.
  bool CreateAlignedWall(float x, float y, Wall::Type type);

  // Works only with grid map.
  bool Blow(const Vector2f& location);
  bool Morph(const Vector2f& location);

  // Returns one of the spawn positions stored in '_spawn_positions'.
  Vector2f GetRandomSpawn() const;

  // XXX[21.08.2012 xairy]: in WorldManager?
  Shape* LoadShape(const std::string& settings_prefix) const;

private:
  bool _LoadWall(const pugi::xml_node& node);
  bool _LoadChunk(const pugi::xml_node& node);
  bool _LoadSpawn(const pugi::xml_node& node);
  bool _LoadStation(const pugi::xml_node& node);

  bool _LoadWallType(const pugi::xml_attribute& attribute, Wall::Type* output);
  bool _LoadStationType(const pugi::xml_attribute& attribute, Station::Type* output);

  // Works only with grid map.
  bool _CreateAlignedWall(int x, int y, Wall::Type type);

  std::map<uint32_t, Entity*> _static_entities;
  std::map<uint32_t, Entity*> _dynamic_entities;

  std::vector<Vector2f> _spawn_positions;

  enum {
    MAP_NONE,
    MAP_GRID
  } _map_type;

  float _block_size; // Works only with grid map.
  float _bound; // Entities with greater coordinates are destroyed.

  IdManager* _id_manager;
  SettingsManager* _settings;
};

} // namespace bm

#endif // BLOWMORPH_SERVER_WORLD_MANAGER_HPP_
