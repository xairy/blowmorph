#include "entity.hpp"

#include <cmath>

#include <memory>

#include "base/error.hpp"
#include "base/macros.hpp"
#include "base/protocol.hpp"
#include "base/pstdint.hpp"

#include "id_manager.hpp"
#include "vector.hpp"
#include "shape.hpp"
#include "world_manager.hpp"

namespace bm {

using namespace protocol;

// Entity.

Entity::Entity(WorldManager* world_manager, uint32_t id)
  : _world_manager(world_manager), _id(id), _shape(NULL), _is_destroyed(false) { }
Entity::~Entity() {
  if(_shape != NULL) {
    delete _shape;
    _shape = NULL;
  }
}

uint32_t Entity::GetId() const {
  return _id;
}

Shape* Entity::GetShape() {
  return _shape;
}
void Entity::SetShape(Shape* shape) {
  _shape = shape;
}

Vector2 Entity::GetPosition() const {
  return _shape->GetPosition();
}
void Entity::SetPosition(const Vector2& position) {
  _shape->SetPosition(position);
}

void Entity::Destroy() {
  _is_destroyed = true;
}
bool Entity::IsDestroyed() const {
  return _is_destroyed;
}

// Player.

Player* Player::Create(
  WorldManager* world_manager,
  uint32_t id,
  const Vector2& position,
  float speed,
  float size,
  uint32_t fire_delay,
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
  player->_last_event_time = 0;

  player->_last_fire_time = 0;
  player->_fire_delay = fire_delay;

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

bool Player::IsStatic() {
  return false;
}

void Player::Update(uint32_t time) {
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

EntitySnapshot Player::GetSnapshot(uint32_t time) {
  EntitySnapshot result;
  result.type = BM_ENTITY_PLAYER;
  result.time = time;
  result.id = _id;
  result.x = _prev_position.x;
  result.y = _prev_position.y;
  return result;
}

void Player::SetPosition(const Vector2& position) {
  _prev_position = _shape->GetPosition();
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
    if(event.time >= _last_fire_time + _fire_delay) {
      _last_fire_time = event.time;
      Vector2 start = GetPosition();
      Vector2 end(static_cast<float>(event.x), static_cast<float>(event.y));
      if(_world_manager->CreateBullet(_id, start, end, _bullet_speed,
        _bullet_radius, _bullet_explosion_radius, event.time) == false)
      {
        return false;
      }
    }
  }
  return true;
}

float Player::GetSpeed() const {
  return _speed;
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

// Dummy.

Dummy* Dummy::Create(
  WorldManager* world_manager,
  uint32_t id,
  float radius,
  float speed,
  const Vector2& path_center,
  float path_radius
) {
  std::auto_ptr<Dummy> dummy(new Dummy(world_manager, id));
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

Dummy::~Dummy() { }

bool Dummy::IsStatic() {
  return false;
}

void Dummy::Update(uint32_t time) {
  float t = static_cast<float>(time);
  Vector2 position;
  position.x = _path_center.x + _path_radius * sin(t * _speed / 1000.0f);
  position.y = _path_center.y + _path_radius * cos(t * _speed / 1000.0f);
  _shape->SetPosition(position);
}

EntitySnapshot Dummy::GetSnapshot(uint32_t time) {
  EntitySnapshot result;
  result.type = BM_ENTITY_DUMMY;
  result.time = time;
  result.id = _id;
  result.x = _shape->GetPosition().x;
  result.y = _shape->GetPosition().y;
  return result;
}

bool Dummy::Collide(Entity* entity) {
  return entity->Collide(this);
}

bool Dummy::Collide(Player* other) {
  return Entity::Collide(other, this);
}
bool Dummy::Collide(Dummy* other) {
  return Entity::Collide(other, this);
}
bool Dummy::Collide(Bullet* other) {
  return Entity::Collide(this, other);
}
bool Dummy::Collide(Wall* other) {
  return Entity::Collide(other, this);
}

Dummy::Dummy(WorldManager* world_manager, uint32_t id) : Entity(world_manager, id) { }

// Bullet.

Bullet* Bullet::Create(
  WorldManager* world_manager,
  uint32_t id,
  uint32_t owner_id,
  const Vector2& start,
  const Vector2& end,
  float speed,
  float radius,
  float explosion_radius,
  uint32_t time
) {
  std::auto_ptr<Bullet> bullet(new Bullet(world_manager, id));
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

Bullet::~Bullet() { }

bool Bullet::IsStatic() {
  return false;
}

void Bullet::Update(uint32_t time) {
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

EntitySnapshot Bullet::GetSnapshot(uint32_t time) {
  EntitySnapshot result;
  result.type = BM_ENTITY_BULLET;
  result.time = time;
  result.id = _id;
  result.x = _shape->GetPosition().x;
  result.y = _shape->GetPosition().y;
  return result;
}

bool Bullet::IsExploded() const {
  return _state == STATE_EXPLODED;
}

// The bullet will be exploded after the next 'Update()' call.
void Bullet::Explode() {
  _state = STATE_WILL_EXPLODE;
}

// Double dispatch. Collision detection.

bool Bullet::Collide(Entity* entity) {
  return entity->Collide(this);
}

bool Bullet::Collide(Player* other) {
  return Entity::Collide(other, this);
}
bool Bullet::Collide(Dummy* other) {
  return Entity::Collide(other, this);
}
bool Bullet::Collide(Bullet* other) {
  return Entity::Collide(this, other);
}
bool Bullet::Collide(Wall* other) {
  return Entity::Collide(other, this);
}

Bullet::Bullet(WorldManager* world_manager, uint32_t id) : Entity(world_manager, id) { }

// Wall.

Wall* Wall::Create(
  WorldManager* world_manager,
  uint32_t id,
  const Vector2& position,
  float size
) {
  std::auto_ptr<Wall> wall(new Wall(world_manager, id));
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

Wall::~Wall() { }

bool Wall::IsStatic() {
  return true;
}

void Wall::Update(uint32_t time) { }

EntitySnapshot Wall::GetSnapshot(uint32_t time) {
  EntitySnapshot result;
  result.type = BM_ENTITY_WALL;
  result.time = time;
  result.id = _id;
  result.x = _shape->GetPosition().x;
  result.y = _shape->GetPosition().y;
  return result;
}

// Double dispatch. Collision detection.

bool Wall::Collide(Entity* entity) {
  return entity->Collide(this);
}

bool Wall::Collide(Player* other) {
  return Entity::Collide(this, other);
}
bool Wall::Collide(Dummy* other) {
  return Entity::Collide(this, other);
}
bool Wall::Collide(Bullet* other) {
  return Entity::Collide(this, other);
}
bool Wall::Collide(Wall* other) {
  return Entity::Collide(other, this);
}

Wall::Wall(WorldManager* world_manager, uint32_t id) : Entity(world_manager, id) { }

// Double dispatch.

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
