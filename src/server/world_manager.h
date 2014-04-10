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

#include "server/entity.h"

#include "server/bullet.h"
#include "server/dummy.h"
#include "server/player.h"
#include "server/wall.h"
#include "server/station.h"

namespace bm {

/*
b2Shape* LoadShape(SettingsManager* settings, const std::string& prefix) {
  std::string shape_type = settings.GetString(prefix + ".type");
  if (shape_type == "circle") {
    float radius = settings.GetFloat(prefix + ".radius");
    b2CircleShape* shape = new b2CircleShape();
    CHECK(shape != NULL);
    shape->m_radius = radius;
    return shape;
  } else if (shape_type == "square") {
    float side = settings.GetFloat(prefix + ".side");
    Shape* shape = new Square(Vector2f(0.0f, 0.0f), side);
    CHECK(shape != NULL);
    return shape;
  }
  THROW_ERROR("Unknown shape type.");
  return NULL;
}
*/

// !FIXME: move it.
b2Body* CreateBox(b2World* world, b2Vec2 position, b2Vec2 extent, bool dynamic);
b2Body* CreateCircle(b2World* world, b2Vec2 position, float radius, bool dynamic);

class Entity;
class IdManager;

class WorldManager {
 public:
  WorldManager(IdManager* id_manager);
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

  bool CreateBullet(
    uint32_t owner_id,
    const b2Vec2& start,
    const b2Vec2& end,
    int64_t time);

  bool CreateDummy(
    const b2Vec2& position,
    int64_t time);

  bool CreateWall(
    const b2Vec2& position,
    Wall::Type type);

  bool CreateStation(
    const b2Vec2& position,
    int health_regeneration,
    int blow_regeneration,
    int morph_regeneration,
    Station::Type type);

  // Works only with grid map.
  bool CreateAlignedWall(float x, float y, Wall::Type type);

  // Works only with grid map.
  bool Blow(const b2Vec2& location);
  bool Morph(const b2Vec2& location);

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
  bool _CreateAlignedWall(int x, int y, Wall::Type type);

  b2World world_;

  std::map<uint32_t, Entity*> _static_entities;
  std::map<uint32_t, Entity*> _dynamic_entities;

  std::vector<b2Vec2> _spawn_positions;

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
