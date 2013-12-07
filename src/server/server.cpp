// Copyright (c) 2013 Blowmorph Team

#include "server/server.h"

#include <cstdio>
#include <cstring>

#include <algorithm>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <enet-plus/enet.h>

#include "base/error.h"
#include "base/macros.h"
#include "base/protocol.h"
#include "base/pstdint.h"
#include "base/settings_manager.h"
#include "base/timer.h"

#include "server/client_manager.h"
#include "server/entity.h"
#include "server/id_manager.h"
#include "server/vector.h"
#include "server/shape.h"
#include "server/world_manager.h"

#include "server/bullet.h"
#include "server/dummy.h"
#include "server/player.h"
#include "server/wall.h"
#include "server/station.h"

namespace bm {

Server::Server() : _state(STATE_FINALIZED), _host(NULL), _event(NULL),
  _world_manager(&_id_manager, &_settings) { }

Server::~Server() {
  if (_state == STATE_INITIALIZED) {
    Finalize();
  }
}

bool Server::Initialize() {
  CHECK(_state == STATE_FINALIZED);

  if (!_settings.Open("data/server.cfg")) {
    return false;
  }

  _server_port = _settings.GetUInt16("server.port");

  _broadcast_rate = _settings.GetUInt32("server.broadcast_rate");
  _broadcast_time = 1000 / _broadcast_rate;
  _last_broadcast = 0;

  _update_rate = _settings.GetUInt32("server.update_rate");
  _update_time = 1000 / _update_rate;
  _last_update = 0;

  _latency_limit = _settings.GetUInt32("server.latency_limit");

  _host = NULL;
  _event = NULL;

  _map_file = _settings.GetString("server.map");

  if (!_world_manager.LoadMap(_map_file)) {
    return false;
  }

  if (!_enet.Initialize()) {
    return false;
  }

  std::auto_ptr<enet::ServerHost> host(_enet.CreateServerHost(_server_port));
  if (host.get() == NULL) {
    return false;
  }

  std::auto_ptr<enet::Event> event(_enet.CreateEvent());
  if (event.get() == NULL) {
    return false;
  }

  _host = host.release();
  _event = event.release();

  _state = STATE_INITIALIZED;
  return true;
}

void Server::Finalize() {
  CHECK(_state == STATE_INITIALIZED);
  if (_event != NULL) {
    delete _event;
    _event = NULL;
  }
  if (_host != NULL) {
    delete _host;
    _host = NULL;
  }
  _state = STATE_FINALIZED;
}

bool Server::Tick() {
  CHECK(_state == STATE_INITIALIZED);

  if (_timer.GetTime() - _last_broadcast >= _broadcast_time) {
    _last_broadcast = _timer.GetTime();
    if (!_BroadcastDynamicEntities()) {
      return false;
    }
    if (!_BroadcastStaticEntities()) {
      return false;
    }
  }

  if (_timer.GetTime() - _last_update >= _update_time) {
    _last_update = _timer.GetTime();
    if (!_UpdateWorld()) {
      return false;
    }
  }

  if (!_PumpEvents()) {
    return false;
  }

  uint32_t next_broadcast = _last_broadcast + _broadcast_time;
  uint32_t next_update = _last_update + _update_time;
  uint32_t sleep_until = std::min(next_broadcast, next_update);
  uint32_t current_time = _timer.GetTime();

  if (current_time <= sleep_until) {
    bool rv = _host->Service(NULL, sleep_until - current_time);
    if (rv == false) {
      return false;
    }
  } else {
    printf("Can't keep up, %u ms behind!\n", current_time - sleep_until);
  }

  return true;
}

bool Server::_UpdateWorld() {
  // XXX(xairy): Temporary.
  static int counter = 0;
  if (counter == 300) {
    float x = -250.0f + static_cast<float>(rand()) / RAND_MAX * 500.0f;  // NOLINT
    float y = -250.0f + static_cast<float>(rand()) / RAND_MAX * 500.0f;  // NOLINT
    _world_manager.CreateDummy(Vector2f(x, y), _timer.GetTime());
    counter = 0;
    // printf("Dummy spawned at (%.2f, %.2f)\n", x, y);
  }
  counter++;

  _world_manager.UpdateEntities(_timer.GetTime());

  _world_manager.CollideEntities();

  _world_manager.DestroyOutlyingEntities();

  if (!_DeleteDestroyedEntities()) {
    return false;
  }

  return true;
}

bool Server::_DeleteDestroyedEntities() {
  std::vector<uint32_t> destroyed_entities;
  _world_manager.GetDestroyedEntities(&destroyed_entities);

  size_t size = destroyed_entities.size();
  for (size_t i = 0; i < size; i++) {
    uint32_t id = destroyed_entities[i];
    bool rv = _BroadcastEntityRelatedMessage(Packet::TYPE_ENTITY_DISAPPEARED,
      _world_manager.GetEntity(id));
    if (rv == false) {
      return false;
    }
  }

  _world_manager.DeleteEntities(destroyed_entities, true);

  return true;
}

bool Server::_BroadcastStaticEntities(bool force) {
  std::map<uint32_t, Entity*>* _entities =
    _entities = _world_manager.GetStaticEntities();
  std::map<uint32_t, Entity*>::iterator itr, end;
  end = _entities->end();
  for (itr = _entities->begin(); itr != end; ++itr) {
    Entity* entity = itr->second;
    if (force || entity->IsUpdated()) {
      bool rv = _BroadcastEntityRelatedMessage(
          Packet::TYPE_ENTITY_UPDATED, entity);
      if (rv == false) {
        return false;
      }
      // XXX[18.08.2012 xairy]: hack.
      if (entity->GetType() == "Wall") {
        entity->SetUpdatedFlag(false);
      }
    }
  }

  return true;
}

bool Server::_BroadcastDynamicEntities() {
  std::map<uint32_t, Entity*>* _entities =
    _entities = _world_manager.GetDynamicEntities();
  std::map<uint32_t, Entity*>::iterator itr, end;
  end = _entities->end();
  for (itr = _entities->begin(); itr != end; ++itr) {
    Entity* entity = itr->second;
    bool rv = _BroadcastEntityRelatedMessage(
        Packet::TYPE_ENTITY_UPDATED, entity);
    if (rv == false) {
      return false;
    }
  }
  return true;
}

bool Server::_PumpEvents() {
  do {
    // TODO(xairy): timeout.
    if (_host->Service(_event, 0) == false) {
      return false;
    }

    switch (_event->GetType()) {
      case enet::Event::TYPE_CONNECT: {
        _OnConnect();
        break;
      }

      case enet::Event::TYPE_RECEIVE: {
        if (!_OnReceive()) {
          return false;
        }
        break;
      }

      case enet::Event::TYPE_DISCONNECT: {
        if (!_OnDisconnect()) {
          return false;
        }
        break;
      }

      case enet::Event::TYPE_NONE:
        break;
    }
  } while (_event->GetType() != enet::Event::TYPE_NONE);

  return true;
}

void Server::_OnConnect() {
  CHECK(_event->GetType() == enet::Event::TYPE_CONNECT);

  uint32_t client_id = _id_manager.NewId();
  _event->GetPeer()->SetData(reinterpret_cast<void*>(client_id));

  printf("#%u: Client from %s:%u is trying to connect.\n", client_id,
    _event->GetPeer()->GetIp().c_str(), _event->GetPeer()->GetPort());

  // Client should send 'TYPE_LOGIN' packet now.
}

bool Server::_BroadcastEntityRelatedMessage(Packet::Type packet_type,
    Entity* entity) {
  CHECK(packet_type == Packet::TYPE_ENTITY_APPEARED ||
    packet_type == Packet::TYPE_ENTITY_DISAPPEARED ||
    packet_type == Packet::TYPE_ENTITY_UPDATED);

  EntitySnapshot snapshot;
  entity->GetSnapshot(_timer.GetTime(), &snapshot);

  bool rv = _BroadcastPacket(packet_type, snapshot, true);
  if (rv == false) {
    return false;
  }

  // if (packet_type == TYPE_ENTITY_APPEARED) {
  //   printf("Entity %u appeared.\n", entity->GetId());
  // }
  // if (packet_type == TYPE_ENTITY_DISAPPEARED) {
  //   printf("Entity %u disappeared.\n", entity->GetId());
  // }

  return true;
}

bool Server::_OnDisconnect() {
  CHECK(_event->GetType() == enet::Event::TYPE_DISCONNECT);

  void* peer_data = _event->GetPeer()->GetData();
  // So complicated to make it work under x64.
  uint32_t id = static_cast<uint32_t>(reinterpret_cast<size_t>(peer_data));
  Client* client = _client_manager.GetClient(id);

  bool rv = _BroadcastEntityRelatedMessage(Packet::TYPE_ENTITY_DISAPPEARED,
    client->entity);
  if (rv == false) {
    return false;
  }

  client->entity->Destroy();
  _client_manager.DeleteClient(id, true);

  printf("#%u from %s:%u disconnected.\n", id,
    _event->GetPeer()->GetIp().c_str(), _event->GetPeer()->GetPort());

  return true;
}

// FIXME(xairy): fix error messages.
bool Server::_OnReceive() {
  CHECK(_event->GetType() == enet::Event::TYPE_RECEIVE);

  void* peer_data = _event->GetPeer()->GetData();
  // So complicated to make it work under x64.
  uint32_t id = static_cast<uint32_t>(reinterpret_cast<size_t>(peer_data));

  std::vector<char> message;
  _event->GetData(&message);

  Packet::Type packet_type;
  bool rv = _ExtractPacketType(message, &packet_type);
  if (rv == false) {
    printf("#%u: Client dropped due to incorrect message format. [0]\n", id);
    _client_manager.DisconnectClient(id);
    return true;
  }

  if (packet_type == Packet::TYPE_LOGIN) {
    if (!_OnLogin(id)) {
      return false;
    }
    return true;
  }

  Client* client = _client_manager.GetClient(id);

  switch (packet_type) {
    case Packet::TYPE_SYNC_TIME_REQUEST: {
      if (message.size() != sizeof(Packet::Type) + sizeof(TimeSyncData)) {
        printf("#%u: Client dropped due to incorrect message format.5\n", id);
        _client_manager.DisconnectClient(id);
        return true;
      }

      TimeSyncData sync_data;
      rv = _ExtractData(message, &sync_data);
      if (rv == false) {
        printf("#%u: Client dropped due to incorrect message format.6\n", id);
        _client_manager.DisconnectClient(id);
        return true;
      }

      sync_data.server_time = _timer.GetTime();
      packet_type = Packet::TYPE_SYNC_TIME_RESPONSE;

      rv = _SendPacket(client->peer, packet_type, sync_data, true);
      if (rv == false) {
        return false;
      }

      _host->Flush();

      // XXX(xairy): hack?
      if (!_BroadcastStaticEntities(true)) {
        return false;
      }
    } break;

    case Packet::TYPE_CLIENT_STATUS: {
      if (!_OnClientStatus(id)) {
        return false;
      }
    } break;

    case Packet::TYPE_KEYBOARD_EVENT: {
      if (message.size() != sizeof(Packet::Type) + sizeof(KeyboardEvent)) {
        printf("#%u: Client dropped due to incorrect message format.1\n", id);
        _client_manager.DisconnectClient(id);
        return true;
      }

      KeyboardEvent keyboard_event;
      rv = _ExtractData(message, &keyboard_event);
      if (rv == false) {
        printf("#%u: Client dropped due to incorrect message format.2\n", id);
        _client_manager.DisconnectClient(id);
        return true;
      }

      client->entity->OnKeyboardEvent(keyboard_event);
    } break;

    case Packet::TYPE_MOUSE_EVENT: {
      if (message.size() != sizeof(Packet::Type) + sizeof(MouseEvent)) {
        printf("#%u: Client dropped due to incorrect message format.3\n", id);
        _client_manager.DisconnectClient(id);
        return true;
      }

      MouseEvent mouse_event;
      rv = _ExtractData(message, &mouse_event);
      if (rv == false) {
        printf("#%u: Client dropped due to incorrect message format.4\n", id);
        _client_manager.DisconnectClient(id);
        return true;
      }

      if (!client->entity->OnMouseEvent(mouse_event, _timer.GetTime())) {
        return false;
      }
    } break;

    default: {
      printf("#%u: Client dropped due to incorrect message format.7\n", id);
      _client_manager.DisconnectClient(id);
      return true;
    } break;
  }

  return true;
}

bool Server::_OnLogin(uint32_t client_id) {
  enet::Peer* peer = _event->GetPeer();
  CHECK(peer != NULL);

  // Receive login data.

  std::vector<char> message;
  _event->GetData(&message);

  LoginData login_data;
  bool rv = _ExtractData(message, &login_data);
  if (rv == false) {
    printf("#%u: Incorrect message format, client dropped.\n", client_id);
    return true;
  }

  printf("#%u: Login data has been received.\n", client_id);

  // Create player.

  Player* player = Player::Create(
    &_world_manager,
    client_id,
    Vector2f(0.0f, 0.0f));
  if (player == NULL) {
    THROW_ERROR("Unable to create player!");
    return false;
  }
  player->Respawn();

  login_data.login[LoginData::MAX_LOGIN_LENGTH] = '\0';
  std::string login(&login_data.login[0]);
  Client* client = new Client(peer, player, login);
  CHECK(client != NULL);

  _client_manager.AddClient(client_id, client);
  _world_manager.AddEntity(client_id, player);

  if (!_SendClientOptions(client)) {
    return false;
  }

  printf("#%u: Client options has been sent.\n", client_id);

  // Broadcast the new player info.

  // The new player may not receive these notifications, as it will be
  // synchronizing time and ignoring everything else.

  if (!_BroadcastEntityRelatedMessage(Packet::TYPE_ENTITY_APPEARED,
      client->entity)) {
    return false;
  }

  PlayerInfo player_info;
  player_info.id = client_id;
  std::copy(login.c_str(), login.c_str() + login.size() + 1,
      &player_info.login[0]);
  rv = _BroadcastPacket(Packet::TYPE_PLAYER_INFO, player_info, true);
  if (rv == false) {
    return false;
  }

  printf("#%u: New player info broadcasted.\n", client_id);

  printf("#%u: Client from %s:%u connected.\n", client_id,
    _event->GetPeer()->GetIp().c_str(), _event->GetPeer()->GetPort(),
    client_id);

  return true;
}

bool Server::_SendClientOptions(Client* client) {
  ClientOptions options;
  options.id = client->entity->GetId();
  options.speed = client->entity->GetSpeed();
  options.x = client->entity->GetPosition().x;
  options.y = client->entity->GetPosition().y;
  options.max_health = client->entity->GetMaxHealth();
  options.blow_capacity = client->entity->GetBlowCapacity();
  options.morph_capacity = client->entity->GetMorphCapacity();

  Packet::Type packet_type = Packet::TYPE_CLIENT_OPTIONS;

  bool rv = _SendPacket(client->peer, packet_type, options, true);
  if (rv == false) {
    return false;
  }

  return true;
}

bool Server::_OnClientStatus(uint32_t client_id) {
  // Send to the new player all players' info.

  PlayerInfo player_info;
  Client* client = _client_manager.GetClient(client_id);

  std::map<uint32_t, Client*>* clients = _client_manager.GetClients();
  std::map<uint32_t, Client*>::iterator i;

  for (i = clients->begin(); i != clients->end(); i++) {
    player_info.id = i->first;
    std::string& login = i->second->login;
    std::copy(login.c_str(), login.c_str() + login.size() + 1,
        &player_info.login[0]);
    bool rv = _SendPacket(client->peer, Packet::TYPE_PLAYER_INFO,
        player_info, true);
    if (rv == false) {
      return false;
    }
  }

  printf("#%u: Other players' info sent.\n", client_id);

  return true;
}

}  // namespace bm
