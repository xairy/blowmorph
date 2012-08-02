#include <base/error.hpp>
#include <base/macros.hpp>
#include <base/pstdint.hpp>

#include <enet-wrapper/enet.hpp>
#include <enet-wrapper/server_host.hpp>
#include <enet-wrapper/client_host.hpp>
#include <enet-wrapper/event.hpp>

namespace bm {

Enet::Enet() : _state(STATE_CREATED) {
  if(enet_initialize() != 0) {
    bm::Error::Set(bm::Error::TYPE_ENET_INITIALIZE);
  } else {
    _state = STATE_INITIALIZED;
  }
}

Enet::~Enet() {
  if(_state == STATE_INITIALIZED) {
    enet_deinitialize();
  }
}

bool Enet::IsInitialized() const {
  return _state == STATE_INITIALIZED;
}

ServerHost* Enet::CreateServerHost(
  uint16_t port,
  size_t peer_count,
  size_t channel_count,
  uint32_t incoming_bandwith,
  uint32_t outgoing_bandwith
) {
  CHECK(_state == STATE_INITIALIZED);
  ServerHost* server = ServerHost::Create(port, peer_count, channel_count,
    incoming_bandwith, outgoing_bandwith);
  return server;
}

ClientHost* Enet::CreateClientHost(
  size_t channel_count,
  uint32_t incoming_bandwith,
  uint32_t outgoing_bandwith
) {
  CHECK(_state == STATE_INITIALIZED);
  ClientHost* client = ClientHost::Create(channel_count,
    incoming_bandwith, outgoing_bandwith);
  return client;
}

Event* Enet::CreateEvent() {
  Event* event = new Event;
  if(event == NULL) {
    bm::Error::Set(bm::Error::TYPE_MEMORY);
    return NULL;
  }
  return event;
}

} // namespace bm
