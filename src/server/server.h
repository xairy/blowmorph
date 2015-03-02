// Copyright (c) 2013 Blowmorph Team

#ifndef SERVER_SERVER_H_
#define SERVER_SERVER_H_

#include <cstring>

#include <map>
#include <string>
#include <vector>

#include <enet-plus/enet.h>

#include "base/error.h"
#include "base/id_manager.h"
#include "base/macros.h"
#include "base/pstdint.h"
#include "base/timer.h"

#include "engine/protocol.h"

#include "server/client_manager.h"
#include "server/controller.h"
#include "server/entity.h"

namespace bm {

class Server {
 public:
  Server();
  ~Server();

  bool Initialize();
  void Finalize();

  bool Tick();

 private:
  bool BroadcastDynamicEntities();
  bool BroadcastStaticEntities(bool force = false);

  bool BroadcastGameEvents();

  bool PumpEvents();

  void OnConnect();
  bool OnDisconnect();

  bool OnReceive();

  bool OnLogin(uint32_t client_id);
  bool SendClientOptions(Client* client);

  bool OnClientStatus(uint32_t client_id);

  bool BroadcastEntityRelatedMessage(Packet::Type packet_type,
      ServerEntity* entity);

  int64_t broadcast_timeout_;
  int64_t last_broadcast_;

  int64_t update_timeout_;
  int64_t last_update_;

  enet::Enet enet_;
  enet::ServerHost* host_;
  enet::Event* event_;

  IdManager id_manager_;
  Controller controller_;
  ClientManager client_manager_;

  enum {
    STATE_FINALIZED,
    STATE_INITIALIZED
  } state_;

  DISALLOW_COPY_AND_ASSIGN(Server);
};

}  // namespace bm

#endif  // SERVER_SERVER_H_
