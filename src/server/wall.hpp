#ifndef BLOWMORH_SERVER_WALL_HPP_
#define BLOWMORH_SERVER_WALL_HPP_

#include <string>

#include <base/macros.hpp>
#include <base/protocol.hpp>
#include <base/pstdint.hpp>

#include "entity.hpp"
#include "vector.hpp"

namespace bm {

class Wall : public Entity {
  friend class Entity;

public:
  enum Type {
    TYPE_ORDINARY,
    TYPE_UNBREAKABLE,
    TYPE_MORPHED
  };

  static Wall* Create(
    WorldManager* world_manager,
    uint32_t id,
    const Vector2& position,
    float size,
    Type type
  );
  virtual ~Wall();

  virtual std::string GetType();
  virtual bool IsStatic();

  virtual void Update(uint32_t time);
  virtual EntitySnapshot GetSnapshot(uint32_t time);

  virtual void OnEntityAppearance(Entity* entity);
  virtual void OnEntityDisappearance(Entity* entity);

  virtual void Damage();

  // Double dispatch. Collision detection.

  virtual bool Collide(Entity* entity);

  virtual bool Collide(Player* other);
  virtual bool Collide(Dummy* other);
  virtual bool Collide(Bullet* other);
  virtual bool Collide(Wall* other);

protected:
  DISALLOW_COPY_AND_ASSIGN(Wall);
  Wall(WorldManager* world_manager, uint32_t id);

  Type _type;
};

} // namespace bm

#endif // BLOWMORH_SERVER_WALL_HPP_
