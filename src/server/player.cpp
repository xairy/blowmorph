// Copyright (c) 2015 Blowmorph Team

#include "server/player.h"

#include <memory>
#include <string>

#include <Box2D/Box2D.h>

#include "base/error.h"
#include "base/macros.h"
#include "base/pstdint.h"

#include "engine/body.h"
#include "engine/config.h"
#include "engine/protocol.h"

#include "server/controller.h"
#include "server/entity.h"

namespace bm {

Player::Player(
    Controller* controller,
    std::string entity_name,
    uint32_t id,
    const b2Vec2& position
) : ServerEntity(controller, id, Entity::TYPE_PLAYER, entity_name, position,
        Entity::FILTER_PLAYER, Entity::FILTER_ALL & ~Entity::FILTER_PLAYER) {
  auto config = Config::GetInstance()->GetPlayersConfig();
  CHECK(config.count(entity_name) == 1);
  _speed = config.at(entity_name).speed;
  _score = 0;
  _killer_id = ServerEntity::BAD_ID;
  _max_health = config.at(entity_name).health_max;
  _health_regeneration = config.at(entity_name).health_regen;
  _health = _max_health;
  _energy_capacity = config.at(entity_name).energy_max;
  _energy_regeneration = config.at(entity_name).energy_regen;
  _energy = _energy_capacity;
}

Player::~Player() { }

void Player::GetSnapshot(int64_t time, EntitySnapshot* output) {
  ServerEntity::GetSnapshot(time, output);
  output->type = EntitySnapshot::ENTITY_TYPE_PLAYER;
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
  _health += static_cast<int>(delta_time) * _health_regeneration;
  if (_health > _max_health) {
    _health = _max_health;
  }
  _energy += static_cast<int>(delta_time) * _energy_regeneration;
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

void Player::Collide(ServerEntity* entity) {
  entity->Collide(this);
}

void Player::Collide(Activator* other) {
  ServerEntity::Collide(other, this);
}
void Player::Collide(Critter* other) {
  ServerEntity::Collide(this, other);
}
void Player::Collide(Door* other) {
  ServerEntity::Collide(other, this);
}
void Player::Collide(Kit* other) {
  ServerEntity::Collide(other, this);
}
void Player::Collide(Player* other) {
  ServerEntity::Collide(other, this);
}
void Player::Collide(Projectile* other) {
  ServerEntity::Collide(this, other);
}
void Player::Collide(Wall* other) {
  ServerEntity::Collide(other, this);
}

}  // namespace bm
