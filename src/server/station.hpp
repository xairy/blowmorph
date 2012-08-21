#ifndef BLOWMORH_SERVER_STATION_HPP_
#define BLOWMORH_SERVER_STATION_HPP_

#include <string>

#include <base/macros.hpp>
#include <base/protocol.hpp>
#include <base/pstdint.hpp>

#include "entity.hpp"
#include "vector.hpp"

namespace bm {

class Station : public Entity {
  friend class Entity;

public:
  enum Type {
    TYPE_HEALTH,
    TYPE_BLOW,
    TYPE_MORPH,
    TYPE_COMPOSITE
  };

  static Station* Create(
    WorldManager* world_manager,
    uint32_t id,
    const Vector2& position,
    int health_regeneration,
    int blow_regeneration,
    int morph_regeneration,
    Type type
  );
  virtual ~Station();

  virtual std::string GetType();
  virtual bool IsStatic();

  virtual void Update(uint32_t time);
  virtual void GetSnapshot(uint32_t time, EntitySnapshot* output);

  virtual void OnEntityAppearance(Entity* entity);
  virtual void OnEntityDisappearance(Entity* entity);

  virtual void Damage(int damage);

  // Double dispatch. Collision detection.

  virtual bool Collide(Entity* entity);

  virtual bool Collide(Player* other);
  virtual bool Collide(Dummy* other);
  virtual bool Collide(Bullet* other);
  virtual bool Collide(Wall* other);
  virtual bool Collide(Station* other);

protected:
  DISALLOW_COPY_AND_ASSIGN(Station);
  Station(WorldManager* world_manager, uint32_t id);

  int _health_regeneration;
  int _blow_regeneration;
  int _morph_regeneration;

  Type _type;
};

} // namespace bm

#endif // BLOWMORH_SERVER_STATION_HPP_
