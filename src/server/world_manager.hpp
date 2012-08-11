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

class WorldManager {
public:
  WorldManager();
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
  void DestroyOutlyingEntities(float max_coordinate);

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
  bool CreateWall(const Vector2& position, float size);

  // Works only with grid map.
  bool CreateAlignedWall(float x, float y);

  bool LoadWall(const pugi::xml_node& node);
  bool LoadChunk(const pugi::xml_node& node);

  bool Blow(const Vector2& location, float radius);
  bool Morph(const Vector2& location, int radius);

private:
  // Works only with grid map.
  bool _CreateAlignedWall(int x, int y);

  std::map<uint32_t, Entity*> _static_entities;
  std::map<uint32_t, Entity*> _dynamic_entities;

  enum {
    MAP_NONE,
    MAP_ARBITRARY,
    MAP_GRID
  } _map_type;

  // In case of grid map.
  float _block_size;
};

} // namespace bm

#endif // BLOWMORPH_SERVER_WORLD_MANAGER_HPP_
