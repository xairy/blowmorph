// Copyright (c) 2015 Blowmorph Team

#include "server/player.h"

#include <memory>
#include <string>

#include <Box2D/Box2D.h>

#include "base/body.h"
#include "base/error.h"
#include "base/macros.h"
#include "base/protocol.h"
#include "base/pstdint.h"
#include "base/settings_manager.h"

#include "server/world_manager.h"

namespace bm {

Player::Player(
    WorldManager* world_manager,
    uint32_t id,
    const b2Vec2& position
) : Entity(world_manager, id, "player", position, Entity::FILTER_PLAYER,
           Entity::FILTER_ALL & ~Entity::FILTER_PLAYER) {
  SettingsManager* settings = world_manager->GetSettings();
  _speed = settings->GetFloat("player.speed");

  _score = 0;
  _killer_id = Entity::BAD_ID;

  _max_health = settings->GetInt32("player.max_health");
  _health_regeneration = settings->GetInt32("player.health_regeneration");
  _health = _max_health;

  _energy_capacity = settings->GetInt32("player.energy_capacity");
  _energy_regeneration = settings->GetInt32("player.energy_regeneration");
  _energy = _energy_capacity;
}

Player::~Player() { }

Entity::Type Player::GetType() {
  return Entity::TYPE_PLAYER;
}

bool Player::IsStatic() {
  return false;
}

void Player::GetSnapshot(int64_t time, EntitySnapshot* output) {
  output->type = EntitySnapshot::ENTITY_TYPE_PLAYER;
  output->time = time;
  output->id = _id;
  output->x = body_->GetPosition().x;
  output->y = body_->GetPosition().y;
  output->data[0] = _health;
  output->data[1] = _energy;
  output->data[2] = _score;
}

void Player::Damage(int damage, uint32_t source_id) {
  _health -= damage;
  if (_health <= 0) {
    _killer_id = source_id;
  }
}

void Player::OnKeyboardEvent(const KeyboardEvent& event) {
  switch (event.event_type) {
    case KeyboardEvent::EVENT_KEYDOWN: {
      switch (event.key_type) {
        case KeyboardEvent::KEY_UP: {
          if (event.time <= _keyboard_update_time.up) {
            return;
          }
          _keyboard_state.up = true;
          _keyboard_update_time.up = event.time;
          return;
        }
        case KeyboardEvent::KEY_DOWN: {
          if (event.time <= _keyboard_update_time.down) {
            return;
          }
          _keyboard_state.down = true;
          _keyboard_update_time.down = event.time;
          return;
        }
        case KeyboardEvent::KEY_RIGHT: {
          if (event.time <= _keyboard_update_time.right) {
            return;
          }
          _keyboard_state.right = true;
          _keyboard_update_time.right = event.time;
          return;
        }
        case KeyboardEvent::KEY_LEFT: {
          if (event.time <= _keyboard_update_time.left) {
            return;
          }
          _keyboard_state.left = true;
          _keyboard_update_time.left = event.time;
          return;
        }
      }
    }
    case KeyboardEvent::EVENT_KEYUP: {
      switch (event.key_type) {
        case KeyboardEvent::KEY_UP: {
          if (event.time <= _keyboard_update_time.up) {
            return;
          }
          _keyboard_state.up = false;
          _keyboard_update_time.up = event.time;
          return;
        }
        case KeyboardEvent::KEY_DOWN: {
          if (event.time <= _keyboard_update_time.down) {
            return;
          }
          _keyboard_state.down = false;
          _keyboard_update_time.down = event.time;
          return;
        }
        case KeyboardEvent::KEY_RIGHT: {
          if (event.time <= _keyboard_update_time.right) {
            return;
          }
          _keyboard_state.right = false;
          _keyboard_update_time.right = event.time;
          return;
        }
        case KeyboardEvent::KEY_LEFT: {
          if (event.time <= _keyboard_update_time.left) {
            return;
          }
          _keyboard_state.left = false;
          _keyboard_update_time.left = event.time;
          return;
        }
      }
    }
  }
}

float Player::GetSpeed() const {
  return _speed;
}
void Player::SetSpeed(float speed) {
  _speed = speed;
}

void Player::IncScore() {
    _score++;
}
void Player::DecScore() {
    _score--;
}

uint32_t Player::GetKillerId() const {
  return _killer_id;
}

Player::KeyboardState* Player::GetKeyboardState() {
  return &_keyboard_state;
}

void Player::Regenerate(int64_t delta_time) {
  _health += delta_time * _health_regeneration;
  if (_health > _max_health) {
    _health = _max_health;
  }
  _energy += delta_time * _energy_regeneration;
  if (_energy > _energy_capacity) {
    _energy = _energy_capacity;
  }
}

int Player::GetHealth() const {
  return _health;
}
int Player::GetMaxHealth() const {
  return _max_health;
}
int Player::GetHealthRegeneration() const {
  return _health_regeneration;
}

void Player::SetHealth(int health) {
  _health = health;
}
void Player::SetMaxHealth(int max_health) {
  _max_health = max_health;
}
void Player::SetHealthRegeneration(int health_regeneration) {
  _health_regeneration = health_regeneration;
}

int Player::GetEnergy() const {
  return _energy;
}
int Player::GetEnergyCapacity() const {
  return _energy_capacity;
}
int Player::GetEnergyRegeneration() const {
  return _energy_regeneration;
}

void Player::SetEnergy(int charge) {
  _energy = charge;
}
void Player::SetEnergyCapacity(int capacity) {
  _energy_capacity = capacity;
}
void Player::SetEnergyRegeneration(int regeneration) {
  _energy_regeneration = regeneration;
}

void Player::AddHealth(int value) {
  _health += value;
  if (_health > _max_health) {
    _health = _max_health;
  }
}
void Player::AddEnergy(int value) {
  _energy += value;
  if (_energy > _energy_capacity) {
    _energy = _energy_capacity;
  }
}

void Player::RestoreHealth() {
  _health = _max_health;
}
void Player::RestoreEnergy() {
  _energy = _energy_capacity;
}

void Player::Collide(Entity* entity) {
  entity->Collide(this);
}

void Player::Collide(Player* other) {
  Entity::Collide(other, this);
}
void Player::Collide(Dummy* other) {
  Entity::Collide(this, other);
}
void Player::Collide(Bullet* other) {
  Entity::Collide(this, other);
}
void Player::Collide(Wall* other) {
  Entity::Collide(other, this);
}
void Player::Collide(Kit* other) {
  Entity::Collide(other, this);
}

}  // namespace bm
