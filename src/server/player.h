// Copyright (c) 2013 Blowmorph Team

#ifndef SERVER_PLAYER_H_
#define SERVER_PLAYER_H_

#include <string>

#include <Box2D/Box2D.h>

#include "base/macros.h"
#include "base/protocol.h"
#include "base/pstdint.h"

#include "server/entity.h"

namespace bm {

class Player : public Entity {
  friend class Entity;

  struct KeyboardState {
    KeyboardState() : up(false), down(false), right(false), left(false) { }
    bool up;
    bool down;
    bool right;
    bool left;
  };

  // The time of the last update of each key.
  struct KeyboardUpdateTime {
    KeyboardUpdateTime() : up(0), down(0), right(0), left(0) { }
    int64_t up;
    int64_t down;
    int64_t right;
    int64_t left;
  };

 public:
  Player(WorldManager* world_manager, uint32_t id, const b2Vec2& position);
  virtual ~Player();

  virtual Entity::Type GetType();
  virtual bool IsStatic();

  virtual void Update(int64_t time);
  virtual void GetSnapshot(int64_t time, EntitySnapshot* output);

  virtual void OnEntityAppearance(Entity* entity);
  virtual void OnEntityDisappearance(Entity* entity);

  virtual void Damage(int damage);

  void OnKeyboardEvent(const KeyboardEvent& event);
  void OnMouseEvent(const MouseEvent& event, int64_t time);

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

  void AddHealth(int value);
  void AddBlow(int value);
  void AddMorph(int value);

  void RestoreHealth();
  void RestoreBlow();
  void RestoreMorph();

  // Double dispatch. Collision detection.

  virtual void Collide(Entity* entity);

  virtual void Collide(Player* other);
  virtual void Collide(Dummy* other);
  virtual void Collide(Bullet* other);
  virtual void Collide(Wall* other);
  virtual void Collide(Station* other);

 protected:
  float _speed;  // In vertical and horizontal directions.

  int _max_health;
  int _health_regeneration;
  int _health;

  int _blow_capacity;
  int _blow_consumption;
  int _blow_regeneration;  // Points per ms.
  int _blow_charge;

  int _morph_capacity;
  int _morph_consumption;
  int _morph_regeneration;  // Points per ms.
  int _morph_charge;

  int64_t _last_update_time;
  KeyboardState _keyboard_state;
  KeyboardUpdateTime _keyboard_update_time;

 private:
  DISALLOW_COPY_AND_ASSIGN(Player);
};

}  // namespace bm

#endif  // SERVER_PLAYER_H_
