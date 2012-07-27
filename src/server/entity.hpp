#ifndef BLOWMORPH_SERVER_ENTITY_HPP_
#define BLOWMORPH_SERVER_ENTITY_HPP_

#include <memory>

#include "base/error.hpp"
#include "base/macros.hpp"
#include "base/protocol.hpp"
#include "base/pstdint.hpp"

#include "id_manager.hpp"
#include "vector.hpp"
#include "shape.hpp"
//#include "world_manager.hpp"

namespace bm {

using namespace protocol;

class Player;
class Dummy;
class Bullet;
class Wall;

class Entity {
public:
  Entity(uint32_t id) : _id(id), _shape(NULL), _is_destroyed(false) { }
  virtual ~Entity() {
    if(_shape != NULL) {
      delete _shape;
      _shape = NULL;
    }
  }

  virtual bool IsStatic() = 0; 

  virtual void Update(uint32_t time) = 0;
  virtual EntitySnapshot GetSnapshot(uint32_t time) = 0;
  
  virtual uint32_t GetId() const {
    return _id;
  }

  // TODO: const?
  virtual Shape* GetShape() const {
    return _shape;
  }
  virtual void SetShape(Shape* shape) {
    _shape = shape;
  }

  virtual Vector2 GetPosition() const {
    return _shape->GetPosition();
  }
  virtual void SetPosition(const Vector2& position) {
    _shape->SetPosition(position);
  }

  virtual void Destroy() {
    _is_destroyed = true;
  }
  virtual bool IsDestroyed() const {
    return _is_destroyed;
  }

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
    uint32_t id,
    const Vector2& position,
    float speed,
    float size,
    uint32_t fire_delay
  ) {
    std::auto_ptr<Player> player(new Player(id));
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
    player->_last_event_time = 0;
    player->_last_fire_time = 0;
    player->_fire_delay = fire_delay;
    player->_spawn_position = Vector2(0.0f, 0.0f);

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
  virtual ~Player() { }

  virtual bool IsStatic() {
    return false;
  }

  virtual void Update(uint32_t time) {
    _prev_position = _shape->GetPosition();
    Vector2 velocity;
    velocity.x = _keyboard_state.left * (-_speed)
      + _keyboard_state.right * (_speed);
    velocity.y = _keyboard_state.up * (-_speed)
      + _keyboard_state.down * (_speed);
    float delta_time = static_cast<float>(time - _last_update_time);
    _shape->Move(velocity * delta_time);
    _last_update_time = time;
  }

  virtual EntitySnapshot GetSnapshot(uint32_t time) {
    EntitySnapshot result;
    result.type = BM_ENTITY_PLAYER;
    result.time = time;
    result.id = _id;
    result.x = _prev_position.x;
    result.y = _prev_position.y;
    return result;
  }

  virtual void SetPosition(const Vector2& position) {
    _prev_position = _shape->GetPosition();
    _shape->SetPosition(position);
  }

  void OnKeyboardEvent(const KeyboardEvent& event) {
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

  bool CanFire(uint32_t time) const {
    return time >= _last_fire_time + _fire_delay;
  }

  void Fire(uint32_t time) {
    _last_fire_time = time;
  }

  float GetSpeed() const {
    return _speed;
  }

  void SetSpawnPosition(const Vector2& position) {
    _spawn_position = position;
  }

  void Respawn() {
    SetPosition(_spawn_position);
  }

  // Double dispatch. Collision detection.

  virtual bool Collide(Entity* entity) {
    return entity->Collide(this);
  }

  virtual bool Collide(Player* other) {
    return Entity::Collide(other, this);
  }
  virtual bool Collide(Dummy* other) {
    return Entity::Collide(this, other);
  }
  virtual bool Collide(Bullet* other) {
    return Entity::Collide(this, other);
  }
  virtual bool Collide(Wall* other) {
    return Entity::Collide(other, this);
  }

protected:
  DISALLOW_COPY_AND_ASSIGN(Player);
  Player(uint32_t id) : Entity(id) { }

  // Player's position before the last 'Update'.
  Vector2 _prev_position;

  // Speed in vertical and horizontal directions.
  float _speed;

  KeyboardState _keyboard_state;
  KeyboardUpdateTime _keyboard_update_time;

  uint32_t _last_event_time;
  uint32_t _last_update_time;

  uint32_t _fire_delay;
  uint32_t _last_fire_time;

  Vector2 _spawn_position;
};

class Dummy : public Entity {
  friend class Entity;

public:
  static Dummy* Create(
    uint32_t id,
    float radius,
    float speed,
    const Vector2& path_center,
    float path_radius
  ) {
    std::auto_ptr<Dummy> dummy(new Dummy(id));
    if(dummy.get() == NULL) {
      Error::Set(Error::TYPE_MEMORY);
      return NULL;
    }
    std::auto_ptr<Circle> shape(new Circle(path_center, radius));
    if(shape.get() == NULL) {
      Error::Set(Error::TYPE_MEMORY);
      return NULL;
    }

    dummy->_shape = shape.release();
    dummy->_speed = speed;
    dummy->_path_center = path_center;
    dummy->_path_radius = path_radius;

    return dummy.release();
  }

  virtual ~Dummy() { }

  virtual bool IsStatic() {
    return false;
  }

  virtual void Update(uint32_t time) {
    float t = static_cast<float>(time);
    Vector2 position;
    position.x = _path_center.x + _path_radius * sin(t * _speed / 1000.0f);
    position.y = _path_center.y + _path_radius * cos(t * _speed / 1000.0f);
    _shape->SetPosition(position);
  }

  virtual EntitySnapshot GetSnapshot(uint32_t time) {
    EntitySnapshot result;
    result.type = BM_ENTITY_DUMMY;
    result.time = time;
    result.id = _id;
    result.x = _shape->GetPosition().x;
    result.y = _shape->GetPosition().y;
    return result;
  }

  // Double dispatch. Collision detection.

  virtual bool Collide(Entity* entity) {
    return entity->Collide(this);
  }

  virtual bool Collide(Player* other) {
    return Entity::Collide(other, this);
  }
  virtual bool Collide(Dummy* other) {
    return Entity::Collide(other, this);
  }
  virtual bool Collide(Bullet* other) {
    return Entity::Collide(this, other);
  }
  virtual bool Collide(Wall* other) {
    return Entity::Collide(other, this);
  }

protected:
  DISALLOW_COPY_AND_ASSIGN(Dummy);
  Dummy(uint32_t id) : Entity(id) { }

  // Actual speed in any direction.
  float _speed;

  Vector2 _path_center;
  float _path_radius;
};

class Bullet : public Entity {
  friend class Entity;

public:
  static Bullet* Create(
    uint32_t id,
    uint32_t owner_id,
    const Vector2& start,
    const Vector2& end,
    float speed,
    float radius,
    float explosion_radius,
    uint32_t time
  ) {
    std::auto_ptr<Bullet> bullet(new Bullet(id));
    if(bullet.get() == NULL) {
      Error::Set(Error::TYPE_MEMORY);
      return NULL;
    }
    std::auto_ptr<Shape> shape(new Circle(start, radius));
    if(shape.get() == NULL) {
      Error::Set(Error::TYPE_MEMORY);
      return NULL;
    }

    bullet->_shape = shape.release();
    bullet->_owner_id = owner_id;
    bullet->_start = start;
    bullet->_end = end;
    bullet->_speed = speed;
    bullet->_start_time = time;
    bullet->_explosion_radius = explosion_radius;
    bullet->_state = STATE_FIRED;

    return bullet.release();
  }

  virtual ~Bullet() { }

  virtual bool IsStatic() {
    return false;
  }

  virtual void Update(uint32_t time) {
    Vector2 direction = _end - _start;
    float magnitude = direction.Magnitude();
    if(magnitude != 0.0f) {
      float dt = static_cast<float>(time - _start_time);
      _shape->SetPosition(_start + direction / magnitude * dt * _speed);
    }
    if(_state == STATE_WILL_EXPLODE) {
      // TODO: think about a better way to do it.
      Vector2 position = _shape->GetPosition();
      delete _shape;
      _shape = new Circle(position, _explosion_radius);
      // TODO: replace CHECK with error notification.
      CHECK(_shape != NULL);
      _state = STATE_EXPLODED;
    }
  }

  virtual EntitySnapshot GetSnapshot(uint32_t time) {
    EntitySnapshot result;
    result.type = BM_ENTITY_BULLET;
    result.time = time;
    result.id = _id;
    result.x = _shape->GetPosition().x;
    result.y = _shape->GetPosition().y;
    return result;
  }

  virtual bool IsExploded() const {
    return _state == STATE_EXPLODED;
  }

  // The bullet will be exploded after the next 'Update()' call.
  virtual void Explode() {
    _state = STATE_WILL_EXPLODE;
  }

  // Double dispatch. Collision detection.

  virtual bool Collide(Entity* entity) {
    return entity->Collide(this);
  }

  virtual bool Collide(Player* other) {
    return Entity::Collide(other, this);
  }
  virtual bool Collide(Dummy* other) {
    return Entity::Collide(other, this);
  }
  virtual bool Collide(Bullet* other) {
    return Entity::Collide(this, other);
  }
  virtual bool Collide(Wall* other) {
    return Entity::Collide(other, this);
  }

protected:
  DISALLOW_COPY_AND_ASSIGN(Bullet);
  Bullet(uint32_t id) : Entity(id) { }

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
    uint32_t id,
    const Vector2& position,
    float size
  ) {
    std::auto_ptr<Wall> wall(new Wall(id));
    if(wall.get() == NULL) {
      Error::Set(Error::TYPE_MEMORY);
      return NULL;
    }
    std::auto_ptr<Shape> shape(new Square(position, size));
    if(wall.get() == NULL) {
      Error::Set(Error::TYPE_MEMORY);
      return NULL;
    }
    
    wall->_shape = shape.release();

    return wall.release();
  }

  virtual ~Wall() { }

  virtual bool IsStatic() {
    return true;
  }

  virtual void Update(uint32_t time) { }

  virtual EntitySnapshot GetSnapshot(uint32_t time) {
    EntitySnapshot result;
    result.type = BM_ENTITY_WALL;
    result.time = time;
    result.id = _id;
    result.x = _shape->GetPosition().x;
    result.y = _shape->GetPosition().y;
    return result;
  }

  // Double dispatch. Collision detection.

  virtual bool Collide(Entity* entity) {
    return entity->Collide(this);
  }

  virtual bool Collide(Player* other) {
    return Entity::Collide(this, other);
  }
  virtual bool Collide(Dummy* other) {
    return Entity::Collide(this, other);
  }
  virtual bool Collide(Bullet* other) {
    return Entity::Collide(this, other);
  }
  virtual bool Collide(Wall* other) {
    return Entity::Collide(other, this);
  }

protected:
  DISALLOW_COPY_AND_ASSIGN(Wall);
  Wall(uint32_t id) : Entity(id) { }
};

bool Entity::Collide(Wall* wall1, Wall* wall2) {
  return false;
}
bool Entity::Collide(Wall* wall, Player* player) {
  bool result = false;

  Vector2 px(player->_prev_position.x, player->_shape->GetPosition().y);
  Vector2 py(player->_shape->GetPosition().x, player->_prev_position.y);

  Vector2 position = player->_shape->GetPosition();

  player->_shape->SetPosition(px);
  if(player->_shape->Collide(wall->_shape)) {
    position.y = player->_prev_position.y;
    result &= true;
  }

  player->_shape->SetPosition(py);
  if(player->_shape->Collide(wall->_shape)) {
    position.x = player->_prev_position.x;
    result &= true;
  }

  player->_shape->SetPosition(position);

  return result;
}
bool Entity::Collide(Wall* wall, Dummy* dummy) {
  return false;
}
bool Entity::Collide(Wall* wall, Bullet* bullet) {
  if(wall->_shape->Collide(bullet->_shape)) {
    if(bullet->IsExploded()) {
      bullet->Destroy();
      wall->Destroy();
    } else {
      bullet->Explode();
    }
    return true;
  }
  return false;
}
bool Entity::Collide(Player* player1, Player* player2) {
  return false;
}
bool Entity::Collide(Player* player, Dummy* dummy) {
  return false;
}
bool Entity::Collide(Player* player, Bullet* bullet) {
  if(bullet->_owner_id == player->GetId()) {
    return false;
  }
  if(player->_shape->Collide(bullet->_shape)) {
    // TODO: fix it.
    player->Respawn();
    bullet->Destroy();
    return true;
  }
  return false;
}
bool Entity::Collide(Dummy* dummy1, Dummy* dummy2) {
  return false;
}
bool Entity::Collide(Dummy* dummy, Bullet* bullet) {
  if(dummy->_shape->Collide(bullet->_shape)) {
    dummy->Destroy();
    bullet->Destroy();
    return true;
  }
  return false;
}
bool Entity::Collide(Bullet* bullet1, Bullet* bullet2) {
  if(bullet1->_shape->Collide(bullet2->_shape)) {
    bullet1->Destroy();
    bullet2->Destroy();
    return true;
  }
  return false;
}

} // namespace bm

#endif // BLOWMORPH_SERVER_ENTITY_HPP_
