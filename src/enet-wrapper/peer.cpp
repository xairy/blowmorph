#include "enet-wrapper/peer.hpp"

#include <string>

#include <enet/enet.h>

#include "base/error.hpp"
#include "base/macros.hpp"
#include "base/pstdint.hpp"

namespace bm {

bool Peer::Send(
  const char* data,
  size_t length,
  bool reliable,
  uint8_t channel_id
) {
  enet_uint32 flags = 0;
  if(reliable) {
    flags = flags | ENET_PACKET_FLAG_RELIABLE;
  }
  ENetPacket* packet = enet_packet_create(data, length, flags);
  if(packet == NULL) {
    bm::Error::Set(bm::Error::TYPE_ENET_PACKET_CREATE);
    return false;
  }
  if(enet_peer_send(_peer, channel_id, packet) != 0) {
    enet_packet_destroy(packet);
    bm::Error::Set(bm::Error::TYPE_ENET_SEND);
    return false;
  }
  return true;
}

std::string Peer::GetIp() const {
  const size_t buffer_size = 32;
  char buffer[buffer_size];
  if(enet_address_get_host_ip(&_peer->address, buffer, buffer_size) != 0) {
    bm::Error::Set(bm::Error::TYPE_ENET_GET_HOST_IP);
    buffer[0] = 0;
  }
  return std::string(buffer);
}

uint16_t Peer::GetPort() const {
  return _peer->address.port;
}

void Peer::Disconnect() {
  enet_peer_disconnect(_peer, 0);
}

void Peer::Reset() {
  enet_peer_reset(_peer);
}

void Peer::SetData(void* data) {
  _peer->data = data;
}

void* Peer::GetData() const {
  return _peer->data;
}

Peer::Peer(ENetPeer* peer) : _peer(peer) {
  CHECK(peer != NULL);
}

} // namespace bm
