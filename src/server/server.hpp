#ifndef BLOWMORPH_SERVER_SERVER_HPP_
#define BLOWMORPH_SERVER_SERVER_HPP_

#include <cstdio>
#include <cstring>

#include "enet-wrapper/enet.hpp"

#include "base/error.hpp"
#include "base/macros.hpp"
#include "base/protocol.hpp"
#include "base/pstdint.hpp"
#include "base/timer.hpp"

#include "entity.hpp"
#include "client_manager.hpp"
#include "id_manager.hpp"
#include "vector.hpp"
#include "shape.hpp"
#include "world_manager.hpp"

// What would happen if a player is 'Destroy()'ed?

namespace bm {

using namespace protocol;

class Server {
public:
  Server() {
    _is_running = false;
    _server_port = 4242;

    _tickrate = 20;
    _ticktime = 1000 / _tickrate;
    _last_broadcast = 0;

    _update_rate = 100;
    _update_time = 1000 / _update_rate;
    _last_update = 0;

    _latency_limit = 500;
    
    _host = NULL;
    _event = NULL;

    _spawn_position = Vector2(0.0f, 0.0f);

    _player_speed = 0.1f;
    _player_size = 32.0f;

    _bullet_speed = 0.3f;
    _bullet_radius = 5.0f;
    _bullet_explosion_radius = 40.0f;
    _fire_delay = 100;

    _wall_size = 16.0f;

    _max_coordinate = 600.0f;
  }

  ~Server() {
    _Destroy();
  }

  bool Execute() {
    if(!_Initialize()) {
      return false;
    }

    printf("Server started.\n");

    _is_running = true;

    // Map.

    float rs = 600.0f;
    float ws = _wall_size;

    bool rv;


    for(int i = 1; i <= 20; i++) {
      rv = _world_manager.CreateDummy(0.0f, 0.0f, 100.0f, 0.1f * i, 5.0f);
      CHECK(rv);
    }

    for(int i = -5; i <= 5; i++) {
      for(int j = -5; j <= 5; j++) {
        rv = _world_manager.CreateWall(300.0f + i * ws, 0.0f + j * ws, _wall_size);
        CHECK(rv);
      }
    }

    // Map.

    while(_is_running) {
      if(!_Tick()) {
        return false;
      }
    }

    printf("Server finished.\n");

    return true;
  }

private:
  bool _Initialize() {
    std::auto_ptr<ServerHost> host(_enet.CreateServerHost(_server_port));
    if(host.get() == NULL) {
      return false;
    }

    std::auto_ptr<Event> event(_enet.CreateEvent());
    if(event.get() == NULL) {
      return false;
    }

    _host = host.release();
    _event = event.release();

    return true;
  }

  void _Destroy() {
    if(_event != NULL) {
      delete _event;
    }
    if(_host != NULL) {
      delete _host;
    }
  }

  bool _Tick() {
    if(_timer.GetTime() - _last_broadcast >= _ticktime) {
      if(!_BroadcastWorldSnapshot()) {
        return false;
      }
      _last_broadcast = _timer.GetTime();
    }

    // TODO: different timeout for '_PumpEvents'?
    if(_timer.GetTime() - _last_update >= _update_time) {
      if(!_PumpEvents()) {
        return false;
      }
      if(!_UpdateWorld()) {
        return false;
      }
      _last_update = _timer.GetTime();
    }

    _host->Service(NULL, _update_time - (_timer.GetTime() - _last_update));

    return true;
  }

  bool _UpdateWorld() {
    _world_manager.UpdateEntities(_timer.GetTime());

    _world_manager.CollideEntities();

    _world_manager.DestroyOutlyingEntities(_max_coordinate);

    if(!_DeleteDestroyedEntities()) {
      return false;
    }

    return true;
  }

  bool _DeleteDestroyedEntities() {
    std::vector<uint32_t> destroyed_entities;
    _world_manager.GetDestroyedEntities(&destroyed_entities);

    size_t size = destroyed_entities.size();
    for(size_t i = 0; i < size; i++) {
      uint32_t id = destroyed_entities[i];
      bool rv = _BroadcastEntityRelatedMessage(BM_PACKET_ENTITY_DISAPPEARED,
        _world_manager.GetEntity(id));
      if(rv == false) {
        return false;
      }
    }

    _world_manager.DeleteEntities(destroyed_entities, true);

    return true;
  }

  bool _BroadcastWorldSnapshot() {
    // TODO: send only updated static entites info.
    std::map<uint32_t, Entity*>::iterator itr;
    std::map<uint32_t, Entity*>* _entities = NULL;

    _entities = _world_manager.GetStaticEntities();
    for(itr = _entities->begin(); itr != _entities->end(); ++itr) {
      EntitySnapshot snapshot = itr->second->GetSnapshot(_timer.GetTime());
      bool rv = _BroadcastPacket(BM_PACKET_ENTITY_UPDATED, snapshot, false);
      if(rv == false) {
        return false;
      }
    }

    _entities = _world_manager.GetDynamicEntities();
    for(itr = _entities->begin(); itr != _entities->end(); ++itr) {
      EntitySnapshot snapshot = itr->second->GetSnapshot(_timer.GetTime());
      bool rv = _BroadcastPacket(BM_PACKET_ENTITY_UPDATED, snapshot, false);
      if(rv == false) {
        return false;
      }
    }

    return true;
  }

  bool _PumpEvents() {
    do {
      if(_host->Service(_event) == false) {
        return false;
      }

      switch(_event->GetType()) {
        case Event::EVENT_CONNECT: {
          if(!_OnConnect()) {
            return false;
          }
          break;
        }

        case Event::EVENT_RECEIVE: {
          if(!_OnReceive()) {
            return false;
          }
          break;
        }

        case Event::EVENT_DISCONNECT: {
          if(!_OnDisconnect()) {
            return false;
          }
          break;
        }
      }
    } while(_event->GetType() != Event::EVENT_NONE);

    return true;
  }

  bool _OnConnect() {
    CHECK(_event->GetType() == Event::EVENT_CONNECT);

    printf("Client from %s:%u is trying to connect.\n",
      _event->GetPeerIp().c_str(), _event->GetPeerPort());

    std::auto_ptr<Peer> peer(_event->GetPeer());
    if(peer.get() == NULL) {
      return false;
    }

    uint32_t client_id = Singleton<IdManager>::GetInstance()->NewId();
    peer->SetData(reinterpret_cast<void*>(client_id));

    Player* player = Player::Create(client_id, _spawn_position,
      _player_speed, _player_size, _fire_delay);
    if(player == NULL) {
      Error::Set(Error::TYPE_MEMORY);
      return false;
    }
    Client* client = new Client(peer.release(), player);
    if(client == NULL) {
      Error::Set(Error::TYPE_MEMORY);
      return false;
    }
    _client_manager.AddClient(client_id, client);
    _world_manager.AddEntity(client_id, player);

    printf("Client from %s:%u connected. Id #%u assigned.\n",
      _event->GetPeerIp().c_str(), _event->GetPeerPort(), client_id);

    if(!_SendClientOptions(client)) {
      return false;
    }

    //if(!_BroadcastEntityRelatedMessage(BM_PACKET_ENTITY_APPEARED,
    //    client->entity)) {
    //  return false;
    //}

    return true;
  }

  bool _SendClientOptions(Client* client) {
    ClientOptions options;
    options.id = client->entity->GetId();
    options.speed = client->entity->GetSpeed();
    options.x = client->entity->GetPosition().x;
    options.y = client->entity->GetPosition().y;

    PacketType packet_type = BM_PACKET_CLIENT_OPTIONS;

    bool rv = _SendPacket(client->peer, packet_type, options, true);
    if(rv == false) {
      return false;
    }

    printf("#%u: Client options has been sent.\n", client->entity->GetId());

    return true;
  }

  bool _BroadcastEntityRelatedMessage(PacketType packet_type, Entity* entity) {
    CHECK(packet_type == BM_PACKET_ENTITY_APPEARED ||
      packet_type == BM_PACKET_ENTITY_DISAPPEARED ||
      packet_type == BM_PACKET_ENTITY_UPDATED);

    EntitySnapshot snapshot = entity->GetSnapshot(_timer.GetTime());

    bool rv = _BroadcastPacket(packet_type, snapshot, true);
    if(rv == false) {
      return false;
    }

    //if(packet_type == BM_PACKET_ENTITY_APPEARED) {
    //  printf("Entity %u appeared.\n", entity->GetId());
    //}
    //if(packet_type == BM_PACKET_ENTITY_DISAPPEARED) {
    //  printf("Entity %u disappeared.\n", entity->GetId());
    //}

    return true;
  }

  bool _OnDisconnect() {
    CHECK(_event->GetType() == Event::EVENT_DISCONNECT);

    void* peer_data = _event->GetPeerData();
    // So complicated to make it work under x64.
    uint32_t id = static_cast<uint32_t>(reinterpret_cast<size_t>(peer_data));
    Client* client = _client_manager.GetClient(id);

    bool rv = _BroadcastEntityRelatedMessage(BM_PACKET_ENTITY_DISAPPEARED,
      client->entity);
    if(rv == false) {
      return false;
    }
    
    client->entity->Destroy();
    _client_manager.DeleteClient(id, true);

    printf("#%u from %s:%u disconnected.\n", id,
      _event->GetPeerIp().c_str(), _event->GetPeerPort());

    return true;
  }

  bool _OnReceive() {
    CHECK(_event->GetType() == Event::EVENT_RECEIVE);

    void* peer_data = _event->GetPeerData();
    // So complicated to make it work under x64.
    uint32_t id = static_cast<uint32_t>(reinterpret_cast<size_t>(peer_data));
    Client* client = _client_manager.GetClient(id);

    std::vector<char> message;
    _event->GetData(&message);
    _event->DestroyPacket();

    PacketType packet_type;
    bool rv = _ExtractPacketType(message, &packet_type);
    if(rv == false) {
      printf("#%u: Client dropped due to incorrect message format.0\n", id);
      _client_manager.DisconnectClient(id);
      return true;
    }

    if(packet_type == BM_PACKET_KEYBOARD_EVENT) {
      if(message.size() != sizeof(PacketType) + sizeof(KeyboardEvent)) {
        printf("#%u: Client dropped due to incorrect message format.1\n", id);
        _client_manager.DisconnectClient(id);
        return true;
      }

      KeyboardEvent keyboard_event;
      rv = _ExtractData(message, &keyboard_event);
      if(rv == false) {
        printf("#%u: Client dropped due to incorrect message format.2\n", id);
        _client_manager.DisconnectClient(id);
        return true;
      }

      client->entity->OnKeyboardEvent(keyboard_event);
    } else if(packet_type == BM_PACKET_MOUSE_EVENT) {
      if(message.size() != sizeof(PacketType) + sizeof(MouseEvent)) {
        printf("#%u: Client dropped due to incorrect message format.3\n", id);
        _client_manager.DisconnectClient(id);
        return true;
      }

      MouseEvent mouse_event;
      rv = _ExtractData(message, &mouse_event);
      if(rv == false) {
        printf("#%u: Client dropped due to incorrect message format.4\n", id);
        _client_manager.DisconnectClient(id);
        return true;
      }

      if(mouse_event.event_type == MouseEvent::EVENT_KEYDOWN &&
        mouse_event.button_type == MouseEvent::BUTTON_LEFT) {
        if(client->entity->CanFire(mouse_event.time)) {
          client->entity->Fire(mouse_event.time);
          Vector2 start = client->entity->GetPosition();
          Vector2 end(static_cast<float>(mouse_event.x),
            static_cast<float>(mouse_event.y));
          if(_world_manager.CreateBullet(id, start, end, _bullet_speed,
            _bullet_radius, _bullet_explosion_radius,
            _timer.GetTime()) == false)
          {
            return false;
          }
        }
      }
    } else if(packet_type == BM_PACKET_SYNC_TIME_REQUEST) {
      if(message.size() != sizeof(PacketType) + sizeof(TimeSyncData)) {
        printf("#%u: Client dropped due to incorrect message format.5\n", id);
        _client_manager.DisconnectClient(id);
        return true;
      }

      TimeSyncData sync_data;
      rv = _ExtractData(message, &sync_data);
      if(rv == false) {
        printf("#%u: Client dropped due to incorrect message format.6\n", id);
        _client_manager.DisconnectClient(id);
        return true;
      }

      sync_data.server_time = _timer.GetTime();
      packet_type = BM_PACKET_SYNC_TIME_RESPONSE;

      rv = _SendPacket(client->peer, packet_type, sync_data, true);
      if(rv == false) {
        return false;
      }

      _host->Flush();

      printf("#%u: Time syncronized: client: %u, server: %u.\n",
        client->entity->GetId(), sync_data.client_time, sync_data.server_time);
    } else {
      printf("#%u: Client dropped due to incorrect message format.7\n", id);
      _client_manager.DisconnectClient(id);
      return true;
    }

    return true;
  }

  template<class T>
  bool _BroadcastPacket(PacketType packet_type, const T& data, bool reliable) {
    std::vector<char> message;
    message.insert(message.end(), reinterpret_cast<const char*>(&packet_type),
      reinterpret_cast<const char*>(&packet_type) + sizeof(packet_type));
    message.insert(message.end(), reinterpret_cast<const char*>(&data),
      reinterpret_cast<const char*>(&data) + sizeof(T));

    bool rv = _host->Broadcast(&message[0], message.size(), reliable);
    if(rv == false) {
      return false;
    }
    return true;
  }

  template<class T>
  bool _SendPacket(Peer* peer, PacketType type, const T& data, bool reliable) {
    std::vector<char> message;
    message.insert(message.end(), reinterpret_cast<const char*>(&type),
      reinterpret_cast<const char*>(&type) + sizeof(type));
    message.insert(message.end(), reinterpret_cast<const char*>(&data),
      reinterpret_cast<const char*>(&data) + sizeof(T));

    bool rv = peer->Send(&message[0], message.size(), reliable);
    if(rv == false) {
      return false;
    }
    return true;
  }

  // Returns 'false' when message format is incorrect.
  bool _ExtractPacketType(const std::vector<char>& message, PacketType* type) {
    if(message.size() < sizeof(PacketType)) {
      return false;
    }
    memcpy(type, &message[0], sizeof(type));
    return true;
  }

  // Returns 'false' when message format is incorrect.
  template<class T>
  bool _ExtractData(const std::vector<char>& message, T* data) {
    if(message.size() != sizeof(PacketType) + sizeof(T)) {
      return false;
    }
    memcpy(data, &message[0] + sizeof(PacketType), sizeof(T));
    return true;
  }

  bool _is_running;
  uint16_t _server_port;

  uint32_t _tickrate;
  uint32_t _ticktime;
  uint32_t _last_broadcast;

  uint32_t _update_rate;
  uint32_t _update_time;
  uint32_t _last_update;

  uint32_t _latency_limit;

  Enet _enet;
  ServerHost* _host;
  Event* _event;

  WorldManager _world_manager;
  ClientManager _client_manager;
  Timer _timer;

  Vector2 _spawn_position;

  float _player_speed;
  float _player_size;

  float _bullet_speed;
  float _bullet_radius;
  float _bullet_explosion_radius;
  uint32_t _fire_delay;

  float _wall_size;

  // Entities with x or y coordinate more than '_max_coordinate'
  // will be destroyed.
  float _max_coordinate;
};

} // namespace bm

#endif // BLOWMORPH_SERVER_SERVER_HPP_
