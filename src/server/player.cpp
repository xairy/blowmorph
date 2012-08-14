#include "player.hpp"

#include <memory>
#include <string>

#include <base/error.hpp>
#include <base/macros.hpp>
#include <base/protocol.hpp>
#include <base/pstdint.hpp>

#include "vector.hpp"
#include "shape.hpp"
#include "world_manager.hpp"

namespace bm {

Player* Player::Create(
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
) {
  std::auto_ptr<Player> player(new Player(world_manager, id));
  if(player.get() == NULL) {
    Error::Set(Error::TYPE_MEMORY);
    return NULL;
  }
  std::auto_ptr<Shape> shape(new Square(position, size));
  if(shape.get() == NULL) {
    Error::Set(Error::TYPE_MEMORY);
    return NULL;
  }

  player->_shape = shape.release();
  player->_prev_position = position;
  player->_speed = speed;
  player->_last_update_time = 0;

  player->_health = max_health;
  player->_max_health = max_health;
  player->_health_regeneration = health_regeneration;

  player->_blow_charge = blow_capacity;
  player->_blow_capacity = blow_capacity;
  player->_blow_consumption = blow_consumption;
  player->_blow_regeneration = blow_regeneration;

  player->_morph_charge = morph_capacity;
  player->_morph_capacity = morph_capacity;
  player->_morph_consumption = morph_consumption;
  player->_morph_regeneration = morph_regeneration;

  player->_bullet_radius = bullet_radius;
  player->_bullet_speed = bullet_speed;
  player->_bullet_explosion_radius = bullet_explosion_radius;

  player->_keyboard_state.up = false;
  player->_keyboard_state.down = false;
  player->_keyboard_state.left = false;
  player->_keyboard_state.right = false;
  player->_keyboard_update_time.up = 0;
  player->_keyboard_update_time.down = 0;
  player->_keyboard_update_time.right = 0;
  player->_keyboard_update_time.left = 0;

  return player.release();
}
Player::~Player() { }

std::string Player::GetType() {
  return "Player";
}
bool Player::IsStatic() {
  return false;
}

void Player::Update(uint32_t time) {
  uint32_t delta_time = time - _last_update_time;
  _last_update_time = time;

  _prev_position = _shape->GetPosition();
  Vector2 velocity;
  velocity.x = _keyboard_state.left * (-_speed)
    + _keyboard_state.right * (_speed);
  velocity.y = _keyboard_state.up * (-_speed)
    + _keyboard_state.down * (_speed);
  _shape->Move(velocity * static_cast<float>(delta_time));

  _health += delta_time * _health_regeneration;
  if(_health > _max_health) {
    _health = _max_health;
  }
  _blow_charge += delta_time * _blow_regeneration;
  if(_blow_charge > _blow_capacity) {
    _blow_charge = _blow_capacity;
  }
  _morph_charge += delta_time * _morph_regeneration;
  if(_morph_charge > _morph_capacity) {
    _morph_charge = _morph_capacity;
  }
}

void Player::GetSnapshot(uint32_t time, EntitySnapshot* output) {
  output->type = BM_ENTITY_PLAYER;
  output->time = time;
  output->id = _id;
  output->x = _prev_position.x;
  output->y = _prev_position.y;
  output->data[0] = _health;
  output->data[1] = _blow_charge;
  output->data[2] = _morph_charge;
}

void Player::OnEntityAppearance(Entity* entity) {

}
void Player::OnEntityDisappearance(Entity* entity) {

}

void Player::Damage() {
  Respawn();
}

void Player::SetPosition(const Vector2& position) {
  _prev_position = position;
  _shape->SetPosition(position);
}

void Player::OnKeyboardEvent(const KeyboardEvent& event) {
  switch(event.event_type) {
    case KeyboardEvent::EVENT_KEYDOWN: {
      switch(event.key_type) {
        case KeyboardEvent::KEY_UP: {
          if(event.time <= _keyboard_update_time.up) {
            return;
          }
          _keyboard_state.up = true;
          _keyboard_update_time.up = event.time;
          return;
        }
        case KeyboardEvent::KEY_DOWN: {
          if(event.time <= _keyboard_update_time.down) {
            return;
          }
          _keyboard_state.down = true;
          _keyboard_update_time.down = event.time;
          return;
        }
        case KeyboardEvent::KEY_RIGHT: {
          if(event.time <= _keyboard_update_time.right) {
            return;
          }
          _keyboard_state.right = true;
          _keyboard_update_time.right = event.time;
          return;
        }
        case KeyboardEvent::KEY_LEFT: {
          if(event.time <= _keyboard_update_time.left) {
            return;
          }
          _keyboard_state.left = true;
          _keyboard_update_time.left = event.time;
          return;
        }
      }
    }
    case KeyboardEvent::EVENT_KEYUP: {
      switch(event.key_type) {
        case KeyboardEvent::KEY_UP: {
          if(event.time <= _keyboard_update_time.up) {
            return;
          }
          _keyboard_state.up = false;
          _keyboard_update_time.up = event.time;
          return;
        }
        case KeyboardEvent::KEY_DOWN: {
          if(event.time <= _keyboard_update_time.down) {
            return;
          }
          _keyboard_state.down = false;
          _keyboard_update_time.down = event.time;
          return;
        }
        case KeyboardEvent::KEY_RIGHT: {
          if(event.time <= _keyboard_update_time.right) {
            return;
          }
          _keyboard_state.right = false;
          _keyboard_update_time.right = event.time;
          return;
        }
        case KeyboardEvent::KEY_LEFT: {
          if(event.time <= _keyboard_update_time.left) {
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

bool Player::OnMouseEvent(const MouseEvent& event) {
  if(event.event_type == MouseEvent::EVENT_KEYDOWN &&
    event.button_type == MouseEvent::BUTTON_LEFT) {
    if(_blow_charge >= _blow_consumption) {
      _blow_charge -= _blow_consumption;
      Vector2 start = GetPosition();
      Vector2 end(static_cast<float>(event.x), static_cast<float>(event.y));
      if(_world_manager->CreateBullet(_id, start, end, _bullet_speed,
        _bullet_radius, _bullet_explosion_radius, event.time) == false)
      {
        return false;
      }
    }
  }
  if(event.event_type == MouseEvent::EVENT_KEYDOWN &&
    event.button_type == MouseEvent::BUTTON_RIGHT) {
    if(_morph_charge >= _morph_consumption) {
      _morph_charge -= _morph_consumption;
      float x = static_cast<float>(event.x);
      float y = static_cast<float>(event.y);
      if(_world_manager->Morph(Vector2(x, y), 4) == false) {
        return false;
      }
    }
  }
  return true;
}

void Player::Respawn() {
  Vector2 spawn_position = _world_manager->GetRandomSpawn();
  SetPosition(spawn_position);
}

float Player::GetSpeed() const {
  return _speed;
}
void Player::SetSpeed(float speed) {
  _speed = speed;
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

int Player::GetBlowCharge() const {
  return _blow_charge;
}
int Player::GetBlowCapacity() const {
  return _blow_capacity;
}
int Player::GetBlowRegeneration() const {
  return _blow_regeneration;
}

void Player::SetBlowCharge(int charge) {
  _blow_charge = charge;
}
void Player::SetBlowCapacity(int capacity) {
  _blow_capacity = capacity;
}
void Player::SetBlowRegeneration(int regeneration) {
  _blow_regeneration = regeneration;
}

int Player::GetMorphCharge() const {
  return _morph_charge;
}
int Player::GetMorphCapacity() const {
  return _morph_capacity;
}
int Player::GetMorphRegeneration() const {
  return _morph_regeneration;
}

void Player::SetMorphCharge(int charge) {
  _morph_charge = charge;
}
void Player::SetMorphCapacity(int capacity) {
  _morph_capacity = capacity;
}
void Player::SetMorphRegeneration(int regeneration) {
  _morph_regeneration = regeneration;
}

bool Player::Collide(Entity* entity) {
  return entity->Collide(this);
}

bool Player::Collide(Player* other) {
  return Entity::Collide(other, this);
}
bool Player::Collide(Dummy* other) {
  return Entity::Collide(this, other);
}
bool Player::Collide(Bullet* other) {
  return Entity::Collide(this, other);
}
bool Player::Collide(Wall* other) {
  return Entity::Collide(other, this);
}

Player::Player(WorldManager* world_manager, uint32_t id) : Entity(world_manager, id) { }

} // namespace bm
