#ifndef BLOWMORPH_SERVER_ENTITY_HPP_
#define BLOWMORPH_SERVER_ENTITY_HPP_

#include <cmath>

#include <string>

#include <base/macros.hpp>
#include <base/protocol.hpp>
#include <base/pstdint.hpp>

#include "vector.hpp"
#include "shape.hpp"

namespace bm {

using namespace protocol;

class WorldManager;

class Player;
class Dummy;
class Bullet;
class Wall;

class Entity {
public:
  Entity(WorldManager* world_manager, uint32_t id);
  virtual ~Entity();

  virtual std::string GetType() = 0;
  virtual bool IsStatic() = 0; 

  virtual void Update(uint32_t time) = 0;
  virtual EntitySnapshot GetSnapshot(uint32_t time) = 0;

  virtual void OnEntityAppearance(Entity* entity) = 0;
  virtual void OnEntityDisappearance(Entity* entity) = 0;
  
  virtual uint32_t GetId() const;

  virtual Shape* GetShape();
  virtual void SetShape(Shape* shape);

  virtual Vector2 GetPosition() const;
  virtual void SetPosition(const Vector2& position) ;

  virtual void Destroy();
  virtual bool IsDestroyed() const;

  // Double dispatch. Collision detection.

  virtual bool Collide(Entity* entity) = 0;

  virtual bool Collide(Player* other) = 0;
  virtual bool Collide(Dummy* other) = 0;
  virtual bool Collide(Bullet* other) = 0;
  virtual bool Collide(Wall* other) = 0;

  static bool Collide(Wall* wall1, Wall* wall2);
  static bool Collide(Wall* wall, Player* player2);
  static bool Collide(Wall* wall, Dummy* dummy);
  static bool Collide(Wall* wall, Bullet* bullet);
  static bool Collide(Player* player1, Player* player2);
  static bool Collide(Player* player, Dummy* dummy);
  static bool Collide(Player* player, Bullet* bullet);
  static bool Collide(Dummy* dummy1, Dummy* dummy2);
  static bool Collide(Dummy* dummy, Bullet* bullet);
  static bool Collide(Bullet* bullet1, Bullet* bullet2);

protected:
  WorldManager* _world_manager;

  uint32_t _id;
  Shape* _shape;
  bool _is_destroyed;  
};

class Player : public Entity {
  friend class Entity;

  struct KeyboardState {
    bool up;
    bool down;
    bool right;
    bool left;
  };

  // The time of the last update of each key.
  struct KeyboardUpdateTime {
    uint32_t up;
    uint32_t down;
    uint32_t right;
    uint32_t left;
  };

public:
  static Player* Create(
    WorldManager* world_manager,
    uint32_t id,
    const Vector2& position,
    float speed,
    float size,
    uint32_t fire_delay,
    float bullet_radius,
    float bullet_speed,
    float bullet_explosion_radius
  );
  virtual ~Player();

  virtual std::string GetType();
  virtual bool IsStatic();

  virtual void Update(uint32_t time);
  virtual EntitySnapshot GetSnapshot(uint32_t time);

  virtual void OnEntityAppearance(Entity* entity);
  virtual void OnEntityDisappearance(Entity* entity);

  virtual void SetPosition(const Vector2& position);

  void OnKeyboardEvent(const KeyboardEvent& event);
  bool OnMouseEvent(const MouseEvent& event);

  float GetSpeed() const;

  void SetSpawnPosition(const Vector2& position);
  void Respawn();

  // Double dispatch. Collision detection.

  virtual bool Collide(Entity* entity);

  virtual bool Collide(Player* other);
  virtual bool Collide(Dummy* other);
  virtual bool Collide(Bullet* other);
  virtual bool Collide(Wall* other);

protected:
  DISALLOW_COPY_AND_ASSIGN(Player);
  Player(WorldManager* world_manager, uint32_t id);

  // Player's position before the last 'Update'.
  Vector2 _prev_position;

  // Speed in vertical and horizontal directions.
  float _speed;

  Vector2 _spawn_position;

  KeyboardState _keyboard_state;
  KeyboardUpdateTime _keyboard_update_time;

  uint32_t _last_event_time;
  uint32_t _last_update_time;

  uint32_t _fire_delay;
  uint32_t _last_fire_time;

  // Bullet information.
  float _bullet_radius;
  float _bullet_speed;
  float _bullet_explosion_radius;
};

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
};

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

  // The bullet will be exploded after the next 'Update()' call.
  virtual void Explode();
  virtual bool IsExploded() const;

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

  enum {
    STATE_FIRED,
    STATE_WILL_EXPLODE,
    STATE_EXPLODED
  } _state;
};

class Wall : public Entity {
  friend class Entity;

public:
  static Wall* Create(
    WorldManager* world_manager,
    uint32_t id,
    const Vector2& position,
    float size
  );
  virtual ~Wall();

  virtual std::string GetType();
  virtual bool IsStatic();

  virtual void Update(uint32_t time);
  virtual EntitySnapshot GetSnapshot(uint32_t time);

  virtual void OnEntityAppearance(Entity* entity);
  virtual void OnEntityDisappearance(Entity* entity);

  // Double dispatch. Collision detection.

  virtual bool Collide(Entity* entity);

  virtual bool Collide(Player* other);
  virtual bool Collide(Dummy* other);
  virtual bool Collide(Bullet* other);
  virtual bool Collide(Wall* other);

protected:
  DISALLOW_COPY_AND_ASSIGN(Wall);
  Wall(WorldManager* world_manager, uint32_t id);
};

} // namespace bm

#endif // BLOWMORPH_SERVER_ENTITY_HPP_
