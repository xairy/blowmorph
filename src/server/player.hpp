#ifndef BLOWMORPH_SERVER_PLAYER_HPP_
#define BLOWMORPH_SERVER_PLAYER_HPP_

#include <string>

#include <base/macros.hpp>
#include <base/protocol.hpp>
#include <base/pstdint.hpp>

#include "entity.hpp"
#include "vector.hpp"

namespace bm {

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
  // TODO: add Player::Data structure and pass it to the constructor.
  // XXX: create Shape in WorldManager? What to do with bullet creation?
  static Player* Create(
    WorldManager* world_manager,
    uint32_t id,
    const Vector2& position,
    float speed,
    float size,
    int max_health,
    int health_regeneration,
    int blow_capacity,
    int blow_consumption,
    int blow_regeneration,
    int morph_capacity,
    int morph_consumption,
    int morph_regeneration,
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

  virtual void Damage();

  virtual void SetPosition(const Vector2& position);

  void OnKeyboardEvent(const KeyboardEvent& event);
  bool OnMouseEvent(const MouseEvent& event);

  void SetSpawnPosition(const Vector2& position);
  void Respawn();

  float GetSpeed() const;

  int GetMaxHealth() const;

  int GetBlowCharge() const;
  int GetBlowCapacity() const;

  int GetMorphCharge() const;
  int GetMorphCapacity() const;

  // Double dispatch. Collision detection.

  virtual bool Collide(Entity* entity);

  virtual bool Collide(Player* other);
  virtual bool Collide(Dummy* other);
  virtual bool Collide(Bullet* other);
  virtual bool Collide(Wall* other);

protected:
  DISALLOW_COPY_AND_ASSIGN(Player);
  Player(WorldManager* world_manager, uint32_t id);

  Vector2 _prev_position; // Before the last 'Update'.
  Vector2 _spawn_position;

  float _speed; // In vertical and horizontal directions.

  int _health;
  int _max_health;
  int _health_regeneration;

  int _blow_charge;
  int _blow_capacity;
  int _blow_consumption;
  int _blow_regeneration; // Points per ms.

  int _morph_charge;
  int _morph_capacity;
  int _morph_consumption;
  int _morph_regeneration; // Points per ms.

  float _bullet_radius;
  float _bullet_speed;
  float _bullet_explosion_radius;

  uint32_t _last_update_time;
  KeyboardState _keyboard_state;
  KeyboardUpdateTime _keyboard_update_time;
};

} // namespace bm

#endif // BLOWMORPH_SERVER_PLAYER_HPP_
