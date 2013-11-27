// Copyright (c) 2013 Blowmorph Team

#ifndef SERVER_SERVER_H_
#define SERVER_SERVER_H_

#include <cstring>

#include <map>
#include <string>
#include <vector>

#include <enet-plus/enet.hpp>

#include "base/error.h"
#include "base/macros.h"
#include "base/protocol.h"
#include "base/pstdint.h"
#include "base/settings_manager.h"
#include "base/timer.h"

#include "server/client_manager.h"
#include "server/entity.h"
#include "server/id_manager.h"
#include "server/world_manager.h"

namespace bm {

class Server {
 public:
  Server();
  ~Server();

  bool Initialize();
  void Finalize();

  bool Tick();

 private:
  bool _UpdateWorld();

  bool _DeleteDestroyedEntities();

  bool _BroadcastStaticEntities(bool force = false);
  bool _BroadcastDynamicEntities();

  bool _PumpEvents();

  void _OnConnect();

  bool _BroadcastEntityRelatedMessage(Packet::Type packet_type, Entity* entity);

  bool _OnDisconnect();

  bool _OnReceive();

  bool _OnLogin(uint32_t client_id);

  bool _SendClientOptions(Client* client);

  bool _OnClientStatus(uint32_t client_id);

  // TODO(xairy): move it outside of 'Server' class.
  template<class T>
  bool _BroadcastPacket(Packet::Type packet_type,
      const T& data, bool reliable) {
    std::vector<char> message;
    message.insert(message.end(), reinterpret_cast<const char*>(&packet_type),
      reinterpret_cast<const char*>(&packet_type) + sizeof(packet_type));
    message.insert(message.end(), reinterpret_cast<const char*>(&data),
      reinterpret_cast<const char*>(&data) + sizeof(T));

    bool rv = _host->Broadcast(&message[0], message.size(), reliable);
    if (rv == false) {
      THROW_ERROR("Couldn't broadcast packet.");
      return false;
    }
    return true;
  }

  template<class T>
  bool _SendPacket(enet::Peer* peer, Packet::Type type,
      const T& data, bool reliable) {
    std::vector<char> message;
    message.insert(message.end(), reinterpret_cast<const char*>(&type),
      reinterpret_cast<const char*>(&type) + sizeof(type));
    message.insert(message.end(), reinterpret_cast<const char*>(&data),
      reinterpret_cast<const char*>(&data) + sizeof(T));

    bool rv = peer->Send(&message[0], message.size(), reliable);
    if (rv == false) {
      THROW_ERROR("Couldn't send packet.");
      return false;
    }
    return true;
  }

  // Returns 'false' when message format is incorrect.
  bool _ExtractPacketType(const std::vector<char>& message,
      Packet::Type* type) {
    if (message.size() < sizeof(Packet::Type)) {
      return false;
    }
    memcpy(type, &message[0], sizeof(Packet::Type));
    return true;
  }

  // Returns 'false' when message format is incorrect.
  template<class T>
  bool _ExtractData(const std::vector<char>& message, T* data) {
    if (message.size() != sizeof(Packet::Type) + sizeof(T)) {
      return false;
    }
    memcpy(data, &message[0] + sizeof(Packet::Type), sizeof(T));
    return true;
  }

  uint16_t _server_port;

  uint32_t _broadcast_rate;
  uint32_t _broadcast_time;
  uint32_t _last_broadcast;

  uint32_t _update_rate;
  uint32_t _update_time;
  uint32_t _last_update;

  uint32_t _latency_limit;

  std::string _map_file;

  enet::Enet _enet;
  enet::ServerHost* _host;
  enet::Event* _event;

  IdManager _id_manager;
  WorldManager _world_manager;
  ClientManager _client_manager;
  Timer _timer;

  SettingsManager _settings;

  enum {
    STATE_FINALIZED,
    STATE_INITIALIZED
  } _state;

  DISALLOW_COPY_AND_ASSIGN(Server);
};

}  // namespace bm

#endif  // SERVER_SERVER_H_
