// Copyright (c) 2015 Andrey Konovalov

#include "net/enet.h"

#include <enet/enet.h>
#undef CreateEvent  // Windows sucks.

#include "base/macros.h"
#include "base/pstdint.h"

#include "net/host.h"
#include "net/server_host.h"
#include "net/client_host.h"
#include "net/event.h"

namespace bm {

Enet::Enet() : _state(STATE_FINALIZED) { }

Enet::~Enet() {
  if (_state == STATE_INITIALIZED) {
    Finalize();
  }
}

bool Enet::Initialize() {
  CHECK(_state == STATE_FINALIZED);
  if (enet_initialize() != 0) {
    // THROW_ERROR("Unable to initialize enet!");
    return false;
  }
  _state = STATE_INITIALIZED;
  return true;
}

void Enet::Finalize() {
  CHECK(_state == STATE_INITIALIZED);
  enet_deinitialize();
  _state = STATE_FINALIZED;
}

ServerHost* Enet::CreateServerHost(
  uint16_t port,
  size_t peer_count,
  size_t channel_count,
  uint32_t incoming_bandwith,
  uint32_t outgoing_bandwith
) {
  CHECK(_state == STATE_INITIALIZED);
  ServerHost* server = new ServerHost();
  CHECK(server != NULL);
  bool rv = server->Initialize(port, peer_count,
    channel_count, incoming_bandwith, outgoing_bandwith);
  return rv ? server : NULL;
}

ClientHost* Enet::CreateClientHost(
  size_t channel_count,
  uint32_t incoming_bandwith,
  uint32_t outgoing_bandwith
) {
  CHECK(_state == STATE_INITIALIZED);
  ClientHost* client = new ClientHost();
  CHECK(client != NULL);
  bool rv = client->Initialize(channel_count,
    incoming_bandwith, outgoing_bandwith);
  return rv ? client : NULL;
}

Event* Enet::CreateEvent() {
  Event* event = new Event;
  CHECK(event != NULL);
  return event;
}

}  // namespace bm
