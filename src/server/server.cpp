// Copyright (c) 2013 Blowmorph Team

#include "server/server.h"

#include <unistd.h>

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
#include "base/net.h"
#include "base/protocol.h"
#include "base/pstdint.h"
#include "base/settings_manager.h"
#include "base/time.h"

#include "server/client_manager.h"
#include "server/controller.h"
#include "server/entity.h"
#include "server/id_manager.h"

#include "server/bullet.h"
#include "server/critter.h"
#include "server/kit.h"
#include "server/player.h"
#include "server/wall.h"

namespace bm {

Server::Server() : controller_(&id_manager_),
  state_(STATE_FINALIZED), host_(NULL), event_(NULL) { }

Server::~Server() {
  if (state_ == STATE_INITIALIZED) {
    Finalize();
  }
}

bool Server::Initialize() {
  CHECK(state_ == STATE_FINALIZED);

  if (!settings_.Open("data/server.cfg")) {
    return false;
  }

  uint32_t broadcast_rate = settings_.GetUInt32("server.broadcast_rate");
  broadcast_timeout_ = 1000 / broadcast_rate;
  last_broadcast_ = 0;

  uint32_t update_rate = settings_.GetUInt32("server.update_rate");
  update_timeout_ = 1000 / update_rate;
  last_update_ = 0;

  host_ = NULL;
  event_ = NULL;

  std::string map_file = settings_.GetString("server.map");
  if (!controller_.GetWorld()->LoadMap(map_file)) {
    return false;
  }

  if (!enet_.Initialize()) {
    return false;
  }

  uint16_t server_port = settings_.GetUInt16("server.port");
  std::auto_ptr<enet::ServerHost> host(enet_.CreateServerHost(server_port));
  if (host.get() == NULL) {
    return false;
  }

  std::auto_ptr<enet::Event> event(enet_.CreateEvent());
  if (event.get() == NULL) {
    return false;
  }

  host_ = host.release();
  event_ = event.release();

  state_ = STATE_INITIALIZED;
  return true;
}

void Server::Finalize() {
  CHECK(state_ == STATE_INITIALIZED);
  if (event_ != NULL) {
    delete event_;
    event_ = NULL;
  }
  if (host_ != NULL) {
    delete host_;
    host_ = NULL;
  }
  state_ = STATE_FINALIZED;
}

bool Server::Tick() {
  CHECK(state_ == STATE_INITIALIZED);

  if (Timestamp() - last_broadcast_ >= broadcast_timeout_) {
    last_broadcast_ = Timestamp();
    if (!BroadcastDynamicEntities()) {
      return false;
    }
    if (!BroadcastStaticEntities()) {
      return false;
    }
    if (!BroadcastGameEvents()) {
      return false;
    }
  }

  if (Timestamp() - last_update_ >= update_timeout_) {
    last_update_ = Timestamp();
    if (!UpdateWorld()) {
      return false;
    }
  }

  if (!PumpEvents()) {
    return false;
  }

  int64_t next_broadcast = last_broadcast_ + broadcast_timeout_;
  int64_t next_update = last_update_ + update_timeout_;
  int64_t sleep_until = std::min(next_broadcast, next_update);
  int64_t current_time = Timestamp();

  if (current_time <= sleep_until) {
    uint32_t timeout = static_cast<uint32_t>(sleep_until - current_time);
    bool rv = host_->Service(NULL, timeout);
    if (rv == false) {
      return false;
    }
  } else {
    printf("Can't keep up, %ld ms behind!\n", current_time - sleep_until);
  }

  return true;
}

bool Server::BroadcastDynamicEntities() {
  std::map<uint32_t, Entity*>* _entities =
    _entities = controller_.GetWorld()->GetDynamicEntities();
  std::map<uint32_t, Entity*>::iterator itr, end;
  end = _entities->end();
  for (itr = _entities->begin(); itr != end; ++itr) {
    Entity* entity = itr->second;
    bool rv = BroadcastEntityRelatedMessage(
        Packet::TYPE_ENTITY_UPDATED, entity);
    if (rv == false) {
      return false;
    }
  }
  return true;
}

bool Server::BroadcastStaticEntities(bool force) {
  std::map<uint32_t, Entity*>* _entities =
    _entities = controller_.GetWorld()->GetStaticEntities();
  std::map<uint32_t, Entity*>::iterator itr, end;
  end = _entities->end();
  for (itr = _entities->begin(); itr != end; ++itr) {
    Entity* entity = itr->second;
    if (force || entity->IsUpdated()) {
      bool rv = BroadcastEntityRelatedMessage(
          Packet::TYPE_ENTITY_UPDATED, entity);
      if (rv == false) {
        return false;
      }
      entity->SetUpdatedFlag(false);
    }
  }

  return true;
}

bool Server::BroadcastGameEvents() {
  std::vector<GameEvent> *events = controller_.GetGameEvents();
  std::vector<GameEvent>::iterator it;
  for (it = events->begin(); it != events->end(); ++it) {
    bool rv = BroadcastPacket(host_, Packet::TYPE_GAME_EVENT, *it, true);
    if (rv == false) {
      return false;
    }
  }
  events->clear();
  return true;
}

bool Server::UpdateWorld() {
  controller_.Update(Timestamp(), update_timeout_);
  return true;
}

bool Server::PumpEvents() {
  do {
    // TODO(xairy): timeout.
    if (host_->Service(event_, 0) == false) {
      return false;
    }

    switch (event_->GetType()) {
      case enet::Event::TYPE_CONNECT: {
        OnConnect();
        break;
      }

      case enet::Event::TYPE_RECEIVE: {
        if (!OnReceive()) {
          return false;
        }
        break;
      }

      case enet::Event::TYPE_DISCONNECT: {
        if (!OnDisconnect()) {
          return false;
        }
        break;
      }

      case enet::Event::TYPE_NONE:
        break;
    }
  } while (event_->GetType() != enet::Event::TYPE_NONE);

  return true;
}

void Server::OnConnect() {
  CHECK(event_->GetType() == enet::Event::TYPE_CONNECT);

  uint32_t client_id = id_manager_.NewId();
  event_->GetPeer()->SetData(reinterpret_cast<void*>(client_id));

  printf("#%u: Client from %s:%u is trying to connect.\n", client_id,
    event_->GetPeer()->GetIp().c_str(), event_->GetPeer()->GetPort());

  // Client should send 'TYPE_LOGIN' packet now.
}

bool Server::OnDisconnect() {
  CHECK(event_->GetType() == enet::Event::TYPE_DISCONNECT);

  void* peer_data = event_->GetPeer()->GetData();
  // So complicated to make it work under both x32 and x64.
  uint32_t id = static_cast<uint32_t>(reinterpret_cast<size_t>(peer_data));
  Client* client = client_manager_.GetClient(id);

  controller_.OnPlayerDisconnected(client->entity);

  client_manager_.DeleteClient(id, true);

  printf("#%u: Client from %s:%u disconnected.\n", id,
    event_->GetPeer()->GetIp().c_str(), event_->GetPeer()->GetPort());

  return true;
}

bool Server::OnReceive() {
  CHECK(event_->GetType() == enet::Event::TYPE_RECEIVE);

  void* peer_data = event_->GetPeer()->GetData();
  // So complicated to make it work under both x32 and x64.
  uint32_t id = static_cast<uint32_t>(reinterpret_cast<size_t>(peer_data));

  std::vector<char> message;
  event_->GetData(&message);

  Packet::Type packet_type;
  bool rv = ExtractPacketType(message, &packet_type);
  if (rv == false) {
    printf("#%u: Incorrect message format [5], client dropped.\n", id);
    client_manager_.DisconnectClient(id);
    return true;
  }

  if (packet_type == Packet::TYPE_LOGIN) {
    if (!OnLogin(id)) {
      return false;
    }
    return true;
  }

  Client* client = client_manager_.GetClient(id);

  switch (packet_type) {
    case Packet::TYPE_SYNC_TIME_REQUEST: {
      TimeSyncData sync_data;
      rv = ExtractPacketData(message, &sync_data);
      if (rv == false) {
        printf("#%u: Incorrect message format [0], client dropped.\n", id);
        client_manager_.DisconnectClient(id);
        return true;
      }

      sync_data.server_time = Timestamp();
      packet_type = Packet::TYPE_SYNC_TIME_RESPONSE;

      rv = SendPacket(client->peer, packet_type, sync_data, true);
      if (rv == false) {
        return false;
      }

      host_->Flush();
    } break;

    case Packet::TYPE_CLIENT_STATUS: {
      if (!OnClientStatus(id)) {
        return false;
      }
    } break;

    case Packet::TYPE_KEYBOARD_EVENT: {
      KeyboardEvent keyboard_event_;
      rv = ExtractPacketData(message, &keyboard_event_);
      if (rv == false) {
        printf("#%u: Incorrect message format [1], client dropped.\n", id);
        client_manager_.DisconnectClient(id);
        return true;
      }
      controller_.OnKeyboardEvent(client->entity, keyboard_event_);
    } break;

    case Packet::TYPE_MOUSE_EVENT: {
      MouseEvent mouse_event;
      rv = ExtractPacketData(message, &mouse_event);
      if (rv == false) {
        printf("#%u: Incorrect message format [2], client dropped.\n", id);
        client_manager_.DisconnectClient(id);
        return true;
      }
      controller_.OnMouseEvent(client->entity, mouse_event);
    } break;

    case Packet::TYPE_PLAYER_ACTION: {
      PlayerAction action;
      rv = ExtractPacketData(message, &action);
      if (rv == false) {
        printf("#%u: Incorrect message format [3], client dropped.\n", id);
        client_manager_.DisconnectClient(id);
        return true;
      }
      controller_.OnPlayerAction(client->entity, action);
    } break;


    default: {
      printf("#%u: Incorrect message format [4], client dropped.\n", id);
      client_manager_.DisconnectClient(id);
      return true;
    } break;
  }

  return true;
}

bool Server::OnLogin(uint32_t client_id) {
  enet::Peer* peer = event_->GetPeer();
  CHECK(peer != NULL);

  // Receive login data.

  std::vector<char> message;
  event_->GetData(&message);

  LoginData login_data;
  bool rv = ExtractPacketData(message, &login_data);
  if (rv == false) {
    printf("#%u: Incorrect message format [4], client dropped.\n", client_id);
    return true;
  }

  printf("#%u: Login data has been received.\n", client_id);

  // Create player.

  Player* player = controller_.OnPlayerConnected();

  login_data.login[LoginData::MAX_LOGIN_LENGTH] = '\0';
  std::string login(&login_data.login[0]);
  Client* client = new Client(peer, player, login);
  CHECK(client != NULL);
  client_manager_.AddClient(client_id, client);

  if (!SendClientOptions(client)) {
    return false;
  }

  printf("#%u: Client options has been sent.\n", client_id);

  // Broadcast the new player info.

  // The new player may not receive this info, as he will be
  // synchronizing time and ignoring everything else.

  if (!BroadcastEntityRelatedMessage(Packet::TYPE_ENTITY_APPEARED,
      client->entity)) {
    return false;
  }

  PlayerInfo player_info;
  player_info.id = client_id;
  std::copy(login.c_str(), login.c_str() + login.size() + 1,
      &player_info.login[0]);
  rv = BroadcastPacket(host_, Packet::TYPE_PLAYER_INFO, player_info, true);
  if (rv == false) {
    return false;
  }

  printf("#%u: Client from %s:%u connected.\n", client_id,
    event_->GetPeer()->GetIp().c_str(), event_->GetPeer()->GetPort());

  return true;
}

bool Server::SendClientOptions(Client* client) {
  ClientOptions options;
  options.id = client->entity->GetId();
  options.speed = client->entity->GetSpeed();
  options.x = client->entity->GetPosition().x;
  options.y = client->entity->GetPosition().y;
  options.max_health = client->entity->GetMaxHealth();
  options.energy_capacity = client->entity->GetEnergyCapacity();

  Packet::Type packet_type = Packet::TYPE_CLIENT_OPTIONS;

  bool rv = SendPacket(client->peer, packet_type, options, true);
  if (rv == false) {
    return false;
  }

  return true;
}

bool Server::OnClientStatus(uint32_t client_id) {
  // Send to the new player all players' info.

  PlayerInfo player_info;
  Client* client = client_manager_.GetClient(client_id);

  std::map<uint32_t, Client*>* clients = client_manager_.GetClients();
  std::map<uint32_t, Client*>::iterator i;

  for (i = clients->begin(); i != clients->end(); i++) {
    player_info.id = i->first;
    std::string& login = i->second->login;
    std::copy(login.c_str(), login.c_str() + login.size() + 1,
        &player_info.login[0]);
    bool rv = SendPacket(client->peer, Packet::TYPE_PLAYER_INFO,
        player_info, true);
    if (rv == false) {
      return false;
    }
  }

  // And all the static entities.

  if (!BroadcastStaticEntities(true)) {
    return false;
  }

  return true;
}

bool Server::BroadcastEntityRelatedMessage(Packet::Type packet_type,
    Entity* entity) {
  CHECK(packet_type == Packet::TYPE_ENTITY_APPEARED ||
    packet_type == Packet::TYPE_ENTITY_UPDATED);

  EntitySnapshot snapshot;
  entity->GetSnapshot(Timestamp(), &snapshot);

  bool rv = BroadcastPacket(host_, packet_type, snapshot, true);
  if (rv == false) {
    return false;
  }

  return true;
}

}  // namespace bm
