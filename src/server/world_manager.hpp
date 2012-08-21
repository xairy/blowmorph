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

namespace bm {

class Entity;
class IdManager;

class WorldManager {
public:
  WorldManager(IdManager* id_manager);
  ~WorldManager();

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
    const Vector2& start,
    const Vector2& end,
    float speed,
    float radius,
    float explosion_radius,
    uint32_t time
  );
  bool CreateDummy(
    const Vector2& position,
    float radius,
    float speed,
    uint32_t time
  );
  bool CreateWall(
    const Vector2& position,
    float size,
    Wall::Type type
  );

  // Works only with grid map.
  bool CreateAlignedWall(float x, float y, Wall::Type type);

  // Works only with grid map.
  bool Blow(const Vector2& location, float radius);
  bool Morph(const Vector2& location, int radius);

  // Returns one of the spawn positions stored in '_spawn_positions'.
  Vector2 GetRandomSpawn() const;

private:
  bool _LoadWall(const pugi::xml_node& node);
  bool _LoadChunk(const pugi::xml_node& node);
  bool _LoadSpawn(const pugi::xml_node& node);

  bool _LoadWallType(const pugi::xml_attribute& attribute, Wall::Type* output);

  // Works only with grid map.
  bool _CreateAlignedWall(int x, int y, Wall::Type type);

  IdManager* _id_manager;

  std::map<uint32_t, Entity*> _static_entities;
  std::map<uint32_t, Entity*> _dynamic_entities;

  std::vector<Vector2> _spawn_positions;

  enum {
    MAP_NONE,
    MAP_GRID
  } _map_type;

  float _block_size; // Works only with grid map.
  float _bound; // Entities with greater coordinates are destroyed.
};

} // namespace bm

#endif // BLOWMORPH_SERVER_WORLD_MANAGER_HPP_
