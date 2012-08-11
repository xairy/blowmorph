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
  player->_spawn_position = Vector2(0.0f, 0.0f);

  player->_last_update_time = 0;

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
  float delta_time = time - _last_update_time;
  _last_update_time = time;

  _prev_position = _shape->GetPosition();
  Vector2 velocity;
  velocity.x = _keyboard_state.left * (-_speed)
    + _keyboard_state.right * (_speed);
  velocity.y = _keyboard_state.up * (-_speed)
    + _keyboard_state.down * (_speed);
  _shape->Move(velocity * static_cast<float>(delta_time));

  _blow_charge += delta_time * _blow_regeneration;
  if(_blow_charge > _blow_capacity) {
    _blow_charge = _blow_capacity;
  }
  _morph_charge += delta_time * _morph_regeneration;
  if(_morph_charge > _morph_capacity) {
    _morph_charge = _morph_capacity;
  }
}

EntitySnapshot Player::GetSnapshot(uint32_t time) {
  EntitySnapshot result;
  result.type = BM_ENTITY_PLAYER;
  result.time = time;
  result.id = _id;
  result.x = _prev_position.x;
  result.y = _prev_position.y;
  result.data[0] = _blow_charge;
  result.data[1] = _morph_charge;
  return result;
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
      _world_manager->Morph(Vector2(x, y), 4);
    }
  }
  return true;
}

float Player::GetSpeed() const {
  return _speed;
}

int Player::GetBlowCharge() const {
  return _blow_charge;
}
int Player::GetBlowCapacity() const {
  return _blow_capacity;
}
int Player::GetMorphCharge() const {
  return _morph_charge;
}
int Player::GetMorphCapacity() const {
  return _morph_capacity;
}

void Player::SetSpawnPosition(const Vector2& position) {
  _spawn_position = position;
}

void Player::Respawn() {
  SetPosition(_spawn_position);
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
