// Copyright (c) 2015 Andrey Konovalov

#include "net/server_host.h"

#include <enet/enet.h>

#include "base/pstdint.h"

#include "net/event.h"
#include "net/host.h"

namespace bm {

ServerHost::~ServerHost() {
  if (_state == STATE_INITIALIZED) {
    Finalize();
  }
};

bool ServerHost::Initialize(
  uint16_t port,
  size_t peer_count,
  size_t channel_count,
  uint32_t incoming_bandwidth,
  uint32_t outgoing_bandwidth
) {
  bool rv = Host::Initialize("", port, peer_count, channel_count,
    incoming_bandwidth, outgoing_bandwidth);
  if (rv == false) {
    return false;
  }

  _state = STATE_INITIALIZED;
  return true;
}

void ServerHost::Finalize() {
  CHECK(_state == STATE_INITIALIZED);
  _state = STATE_FINALIZED;
};

bool ServerHost::Broadcast(
  const char* data,
  size_t length,
  bool reliable,
  uint8_t channel_id
) {
  CHECK(_state == STATE_INITIALIZED);
  CHECK(data != NULL);
  CHECK(length > 0);

  enet_uint32 flags = 0;
  if (reliable) {
    flags = flags | ENET_PACKET_FLAG_RELIABLE;
  }

  ENetPacket* packet = enet_packet_create(data, length, flags);
  if (packet == NULL) {
    // THROW_ERROR("Unable to create enet packet!");
    return false;
  }

  enet_host_broadcast(_host, channel_id, packet);

  return true;
}

ServerHost::ServerHost() : Host(), _state(STATE_FINALIZED) { }

}  // namespace bm
