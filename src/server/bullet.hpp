#ifndef BLOWMORH_SERVER_BULLET_HPP_
#define BLOWMORH_SERVER_BULLET_HPP_

#include <string>

#include <base/macros.hpp>
#include <base/protocol.hpp>
#include <base/pstdint.hpp>

#include "entity.hpp"
#include "vector.hpp"

namespace bm {

class Bullet : public Entity {
  friend class Entity;

public:
  static Bullet* Create(
    WorldManager* world_manager,
    uint32_t id,
    uint32_t owner_id,
    const Vector2& start,
    const Vector2& end,
    float speed,
    float radius,
    float explosion_radius,
    uint32_t time
  );
  virtual ~Bullet();

  virtual std::string GetType();
  virtual bool IsStatic();

  virtual void Update(uint32_t time);
  virtual EntitySnapshot GetSnapshot(uint32_t time);

  virtual void OnEntityAppearance(Entity* entity);
  virtual void OnEntityDisappearance(Entity* entity);

  virtual void Damage();

  void Explode();

  // Double dispatch. Collision detection.

  virtual bool Collide(Entity* entity) ;

  virtual bool Collide(Player* other);
  virtual bool Collide(Dummy* other);
  virtual bool Collide(Bullet* other);
  virtual bool Collide(Wall* other);

protected:
  DISALLOW_COPY_AND_ASSIGN(Bullet);
  Bullet(WorldManager* world_manager, uint32_t id);

  uint32_t _owner_id;

  // The start and the end of the bullet' trajectory.
  Vector2 _start;
  Vector2 _end;

  uint32_t _start_time;

  // Actual speed in any direction.
  float _speed;

  float _explosion_radius;

  /*enum {
    STATE_FIRED,
    STATE_WILL_EXPLODE,
    STATE_EXPLODED
  } _state;*/
};

} // namespace bm

#endif // BLOWMORH_SERVER_BULLET_HPP_
