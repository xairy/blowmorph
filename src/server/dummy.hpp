#ifndef BLOWMORPH_SERVER_DUMMY_HPP_
#define BLOWMORPH_SERVER_DUMMY_HPP_

#include <string>

#include <base/macros.hpp>
#include <base/protocol.hpp>
#include <base/pstdint.hpp>

#include "entity.hpp"
#include "vector.hpp"

namespace bm {

class Dummy : public Entity {
  friend class Entity;

public:
  static Dummy* Create(
    WorldManager* world_manager,
    uint32_t id,
    const Vector2& position,
    float radius,
    float speed,
    uint32_t time
  );
  virtual ~Dummy();

  virtual std::string GetType();
  virtual bool IsStatic();

  virtual void Update(uint32_t time);
  virtual EntitySnapshot GetSnapshot(uint32_t time);

  virtual void OnEntityAppearance(Entity* entity);
  virtual void OnEntityDisappearance(Entity* entity);

  virtual void Damage();

  virtual void SetPosition(const Vector2& position);

  // Double dispatch. Collision detection.

  virtual bool Collide(Entity* entity);

  virtual bool Collide(Player* other);
  virtual bool Collide(Dummy* other);
  virtual bool Collide(Bullet* other);
  virtual bool Collide(Wall* other);

protected:
  DISALLOW_COPY_AND_ASSIGN(Dummy);
  Dummy(WorldManager* world_manager, uint32_t id);

  float _speed;
  Entity* _meat;
  uint32_t _last_update;
  Vector2 _prev_position;
};

} // namespace bm

#endif // BLOWMORPH_SERVER_DUMMY_HPP_
