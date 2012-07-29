#ifndef BLOWMORPH_SERVER_WORLD_MANAGER_HPP_
#define BLOWMORPH_SERVER_WORLD_MANAGER_HPP_

#include <map>
#include <vector>

#include "base/pstdint.hpp"

#include "entity.hpp"
#include "vector.hpp"

namespace bm {

class WorldManager {
public:
  WorldManager();
  ~WorldManager();

  void AddEntity(uint32_t id, Entity* entity);
  void DeleteEntity(uint32_t id, bool deallocate);

  Entity* GetEntity(uint32_t id);
  std::map<uint32_t, Entity*>* GetStaticEntities();
  std::map<uint32_t, Entity*>* GetDynamicEntities();

  void GetDestroyedEntities(std::vector<uint32_t>* output);

  void DeleteEntities(const std::vector<uint32_t>& input, bool deallocate);
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
  bool CreateDummy(float x, float y, float r, float speed, float radius);
  bool CreateWall(float x, float y, float size);

private:
  std::map<uint32_t, Entity*> _static_entities;
  std::map<uint32_t, Entity*> _dynamic_entities;
};

} // namespace bm

#endif // BLOWMORPH_SERVER_WORLD_MANAGER_HPP_
