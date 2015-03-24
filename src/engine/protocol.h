// Copyright (c) 2015 Blowmorph Team

#ifndef ENGINE_PROTOCOL_H_
#define ENGINE_PROTOCOL_H_

#include "base/macros.h"
#include "base/pstdint.h"

namespace bm {

class Packet {
 public:
  enum Type {
    TYPE_UNKNOWN = 0,

    // C -> S. Followed by 'LoginData'.
    TYPE_LOGIN,

    // S -> C. Followed by 'ClientOptions'.
    TYPE_CLIENT_OPTIONS,

    // C -> S. Followed by 'ClientStatus'.
    TYPE_CLIENT_STATUS,

    // C -> S. Followed by 'TimeSyncData' filled with client time.
    TYPE_SYNC_TIME_REQUEST,
    // S -> C. Followed by 'TimeSyncData' filled with client and server time.
    TYPE_SYNC_TIME_RESPONSE,

    // S -> C. Followed by 'PlayerInfo'.
    TYPE_PLAYER_INFO,

    // S -> C. Followed by 'EntitySnapshot' with the entity description.
    TYPE_ENTITY_APPEARED,  // FIXME(xairy): make a GameEvent.
    TYPE_ENTITY_UPDATED,

    // S -> C. Followed by 'GameEvent'.
    TYPE_GAME_EVENT,

    // C -> S. Followed by 'KeyboardEvent'.
    TYPE_KEYBOARD_EVENT,
    // C -> S. Followed by 'MouseEvent'.
    TYPE_MOUSE_EVENT,

    // C -> S. Followed by 'PlayerAction'.
    TYPE_PLAYER_ACTION,

    TYPE_MAX_VALUE
  };

 private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(Packet);
};

struct LoginData {
  static const size_t MAX_LOGIN_LENGTH = 31;

  char login[MAX_LOGIN_LENGTH + 1];
};

struct ClientOptions {
  uint32_t id;
  float32_t speed;
  float32_t x, y;
  int32_t max_health;
  int32_t energy_capacity;
};

struct TimeSyncData {
  int64_t client_time;
  int64_t server_time;
};

struct ClientStatus {
  enum Status {
    STATUS_SYNCHRONIZED
  };

  Status status;
};

struct PlayerInfo {
  uint32_t id;
  char login[LoginData::MAX_LOGIN_LENGTH + 1];
};

// type == EntitySnapshot::ENTITY_TYPE_PLAYER:
//   data[0] - health
//   data[1] - energy
//   data[2] - score
// type == EntitySnapshot::ENTITY_TYPE_PROJECTILE:
//    data[0] - projectile type
// type == EntitySnapshot::ENTITY_TYPE_WALL:
//    data[0] - wall type
struct EntitySnapshot {
  enum EntityType {
    ENTITY_TYPE_UNKNOWN = 0,

    ENTITY_TYPE_ACTIVATOR,
    ENTITY_TYPE_CRITTER,
    ENTITY_TYPE_DOOR,
    ENTITY_TYPE_KIT,
    ENTITY_TYPE_PLAYER,
    ENTITY_TYPE_PROJECTILE,
    ENTITY_TYPE_WALL,

    ENTITY_TYPE_MAX_VALUE
  };

  enum ProjectileType {
    PROJECTILE_TYPE_UNKNOWN,

    PROJECTILE_TYPE_ROCKET,
    PROJECTILE_TYPE_SLIME,

    PROJECTILE_TYPE_MAX_VALUE
  };

  enum WallType {
    WALL_TYPE_UNKNOWN,

    WALL_TYPE_ORDINARY,
    WALL_TYPE_UNBREAKABLE,
    WALL_TYPE_MORPHED,

    WALL_TYPE_MAX_VALUE
  };

  enum KitType {
    KIT_TYPE_UNKNOWN,

    KIT_TYPE_HEALTH,
    KIT_TYPE_ENERGY,
    KIT_TYPE_COMPOSITE,

    KIT_TYPE_MAX_VALUE
  };

  static const size_t MAX_NAME_LENGTH = 16;

  int64_t time;
  uint32_t id;
  float32_t x, y;
  float32_t angle;

  // FIXME(xairy): use numerical id instead.
  char name[MAX_NAME_LENGTH + 1];
  EntityType type;
  int32_t data[4];
};

struct GameEvent {
  enum EventType {
    TYPE_EXPLOSION,
    TYPE_ENTITY_DISAPPEARED
  };

  EventType type;
  float32_t x, y;
  EntitySnapshot entity;
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

  int64_t time;
  KeyType key_type;
  EventType event_type;
};

struct MouseEvent {
  enum ButtonType {
    BUTTON_NONE,
    BUTTON_LEFT,
    BUTTON_RIGHT
  };
  enum EventType {
    EVENT_KEYDOWN,
    EVENT_KEYUP,
    EVENT_MOVE
  };

  int64_t time;
  EventType event_type;
  ButtonType button_type;
  float32_t x, y;
};

struct PlayerAction {
  enum ActionType {
    TYPE_ACTIVATE
  };

  ActionType type;
  uint32_t target_id;
};

}  // namespace bm

#endif  // ENGINE_PROTOCOL_H_
