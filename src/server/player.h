// Copyright (c) 2015 Blowmorph Team

#ifndef SERVER_PLAYER_H_
#define SERVER_PLAYER_H_

#include <string>

#include <Box2D/Box2D.h>

#include "base/macros.h"
#include "base/protocol.h"
#include "base/pstdint.h"

#include "server/entity.h"

namespace bm {

class Controller;

class Player : public Entity {
  friend class Entity;

 public:
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
  Player(Controller* controller, uint32_t id, const b2Vec2& position);
  virtual ~Player();

  virtual Entity::Type GetType();
  virtual bool IsStatic();

  virtual void GetSnapshot(int64_t time, EntitySnapshot* output);

  virtual void Damage(int damage, uint32_t source_id);

  void OnKeyboardEvent(const KeyboardEvent& event);

  float GetSpeed() const;
  void SetSpeed(float speed);

  void IncScore();
  void DecScore();

  uint32_t GetKillerId() const;

  KeyboardState* GetKeyboardState();

  void Regenerate(int64_t delta_time);

  int GetHealth() const;
  int GetMaxHealth() const;
  int GetHealthRegeneration() const;

  void SetHealth(int health);
  void SetMaxHealth(int max_health);
  void SetHealthRegeneration(int health_regeneration);

  int GetEnergy() const;
  int GetEnergyCapacity() const;
  int GetEnergyRegeneration() const;

  void SetEnergy(int charge);
  void SetEnergyCapacity(int capacity);
  void SetEnergyRegeneration(int regeneration);

  void AddHealth(int value);
  void AddEnergy(int value);

  void RestoreHealth();
  void RestoreEnergy();

  // Double dispatch. Collision detection.

  virtual void Collide(Entity* entity);

  virtual void Collide(Player* other);
  virtual void Collide(Critter* other);
  virtual void Collide(Projectile* other);
  virtual void Collide(Wall* other);
  virtual void Collide(Kit* other);
  virtual void Collide(Activator* other);

 protected:
  float _speed;  // In vertical and horizontal directions.

  int _score;
  uint32_t _killer_id;

  KeyboardState _keyboard_state;
  KeyboardUpdateTime _keyboard_update_time;

  int _max_health;
  int _health_regeneration;  // Points per ms.
  int _health;

  int _energy_capacity;
  int _energy_regeneration;  // Points per ms.
  int _energy;

 private:
  DISALLOW_COPY_AND_ASSIGN(Player);
};

}  // namespace bm

#endif  // SERVER_PLAYER_H_
