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
    TimeType up;
    TimeType down;
    TimeType right;
    TimeType left;
  };

public:
  // XXX: create Shape in WorldManager? What to do with bullet creation?
  static Player* Create(
    WorldManager* world_manager,
    uint32_t id,
    const Vector2f& position
  );
  virtual ~Player();

  virtual std::string GetType();
  virtual bool IsStatic();

  virtual void Update(TimeType time);
  virtual void GetSnapshot(TimeType time, EntitySnapshot* output);

  virtual void OnEntityAppearance(Entity* entity);
  virtual void OnEntityDisappearance(Entity* entity);

  virtual void Damage(int damage);

  virtual void SetPosition(const Vector2f& position);

  void OnKeyboardEvent(const KeyboardEvent& event);
  bool OnMouseEvent(const MouseEvent& event, TimeType time);

  void Respawn();

  float GetSpeed() const;
  void SetSpeed(float speed);

  int GetHealth() const;
  int GetMaxHealth() const;
  int GetHealthRegeneration() const;

  void SetHealth(int health);
  void SetMaxHealth(int max_health);
  void SetHealthRegeneration(int health_regeneration);

  int GetBlowCharge() const;
  int GetBlowCapacity() const;
  int GetBlowRegeneration() const;

  void SetBlowCharge(int charge);
  void SetBlowCapacity(int capacity);
  void SetBlowRegeneration(int regeneration);

  int GetMorphCharge() const;
  int GetMorphCapacity() const;
  int GetMorphRegeneration() const;

  void SetMorphCharge(int charge);
  void SetMorphCapacity(int capacity);
  void SetMorphRegeneration(int regeneration);

  void RestoreHealth(int value);
  void RestoreBlow(int value);
  void RestoreMorph(int value);

  // Double dispatch. Collision detection.

  virtual bool Collide(Entity* entity);

  virtual bool Collide(Player* other);
  virtual bool Collide(Dummy* other);
  virtual bool Collide(Bullet* other);
  virtual bool Collide(Wall* other);
  virtual bool Collide(Station* other);

protected:
  DISALLOW_COPY_AND_ASSIGN(Player);
  Player(WorldManager* world_manager, uint32_t id);

  Vector2f _prev_position; // Before the last 'Update'.

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

  TimeType _last_update_time;
  KeyboardState _keyboard_state;
  KeyboardUpdateTime _keyboard_update_time;
};

} // namespace bm

#endif // BLOWMORPH_SERVER_PLAYER_HPP_
