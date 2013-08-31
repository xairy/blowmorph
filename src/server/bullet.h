#ifndef BLOWMORH_SERVER_BULLET_H_
#define BLOWMORH_SERVER_BULLET_H_

#include <string>

#include <base/macros.h>
#include <base/protocol.h>
#include <base/pstdint.h>

#include "entity.h"
#include "vector.h"

namespace bm {

class Bullet : public Entity {
  friend class Entity;

public:
  static Bullet* Create(
    WorldManager* world_manager,
    uint32_t id,
    uint32_t owner_id,
    const Vector2f& start,
    const Vector2f& end,
    TimeType time
  );
  virtual ~Bullet();

  virtual std::string GetType();
  virtual bool IsStatic();

  virtual void Update(TimeType time);
  virtual void GetSnapshot(TimeType time, EntitySnapshot* output);

  virtual void OnEntityAppearance(Entity* entity);
  virtual void OnEntityDisappearance(Entity* entity);

  virtual void Damage(int damage);

  void Explode();

  // Double dispatch. Collision detection.

  virtual bool Collide(Entity* entity) ;

  virtual bool Collide(Player* other);
  virtual bool Collide(Dummy* other);
  virtual bool Collide(Bullet* other);
  virtual bool Collide(Wall* other);
  virtual bool Collide(Station* other);

protected:
  DISALLOW_COPY_AND_ASSIGN(Bullet);
  Bullet(WorldManager* world_manager, uint32_t id);

  uint32_t _owner_id;

  // The start and the end of the bullet' trajectory.
  Vector2f _start;
  Vector2f _end;

  TimeType _start_time;

  // Actual speed in any direction.
  float _speed;
};

} // namespace bm

#endif // BLOWMORH_SERVER_BULLET_H_
