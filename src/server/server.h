// Copyright (c) 2013 Blowmorph Team

#ifndef SERVER_SERVER_H_
#define SERVER_SERVER_H_

#include <cstring>

#include <map>
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

  uint16_t _server_port;

  uint32_t _broadcast_rate;
  int64_t _broadcast_time;
  int64_t _last_broadcast;

  uint32_t _update_rate;
  int64_t _update_time;
  int64_t _last_update;

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
