#ifndef BLOWMORPH_BASE_PROTOCOL_HPP_
#define BLOWMORPH_BASE_PROTOCOL_HPP_

#include "base/macros.hpp"
#include "base/pstdint.hpp"

namespace bm {

class Packet {
public:
  enum Type {
    TYPE_UNKNOWN = 0,

    // S -> C. Followed by 'ClientOptions'.
    TYPE_CLIENT_OPTIONS,

    // S -> C. Followed by 'EntitySnapshot' with the entity description.
    TYPE_ENTITY_APPEARED,
    TYPE_ENTITY_UPDATED,
    TYPE_ENTITY_DISAPPEARED,

    // C -> S. Followed by 'KeyboardEvent'.
    TYPE_KEYBOARD_EVENT,
    // C -> S. Followed by 'MouseEvent'.
    TYPE_MOUSE_EVENT,

    // C -> S. Followed by 'TimeSyncData' filled with client time.
    TYPE_SYNC_TIME_REQUEST,
    // S -> C. Followed by 'TimeSyncData' filled with client and server time.
    TYPE_SYNC_TIME_RESPONSE,

    TYPE_MAX_VALUE
  };
private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(Packet);
};

struct ClientOptions {
  uint32_t id;
  float32_t speed;
  float32_t x, y;
  int32_t max_health;
  int32_t blow_capacity;
  int32_t morph_capacity;
};

struct TimeSyncData {
  uint32_t client_time;
  uint32_t server_time;
};

// type == EntitySnapshot::ENTITY_TYPE_PLAYER:
//   data[0] - health
//   data[1] - blow charge
//   data[2] - morph charge
// type == EntitySnapshot::ENTITY_TYPE_WALL:
//    data[0] - wall type
struct EntitySnapshot {
  enum EntityType {
    ENTITY_TYPE_UNKNOWN = 0,

    ENTITY_TYPE_PLAYER,
    ENTITY_TYPE_BULLET,
    ENTITY_TYPE_WALL,
    ENTITY_TYPE_DUMMY,

    ENTITY_TYPE_MAX_VALUE
  };

  enum WallType {
    WALL_TYPE_ORDINARY,
    WALL_TYPE_UNBREAKABLE,
    WALL_TYPE_MORPHED
  };

  uint32_t time;
  uint32_t id;
  EntityType type;
  float32_t x;
  float32_t y;
  int32_t data[3];
};

struct KeyboardEvent {
  enum KeyType {
    KEY_UP,
    KEY_DOWN,
    KEY_RIGHT,
    KEY_LEFT
  };
  enum EventType {
    EVENT_KEYDOWN,
    EVENT_KEYUP
  };

  uint32_t time;
  KeyType key_type;
  EventType event_type;
};

struct MouseEvent {
  enum ButtonType {
    BUTTON_LEFT,
    BUTTON_RIGHT
  };
  enum EventType {
    EVENT_KEYDOWN,
    EVENT_KEYUP
  };

  uint32_t time;
  ButtonType button_type;
  EventType event_type;
  float32_t x, y;
};

} //namespace bm

#endif //BLOWMORPH_BASE_PROTOCOL_HPP_
