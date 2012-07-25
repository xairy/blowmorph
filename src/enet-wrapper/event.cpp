#include "enet-wrapper/event.hpp"

#include <string>
#include <vector>

#include <enet/enet.h>

#include "base/error.hpp"
#include "base/pstdint.hpp"

#include "enet-wrapper/peer.hpp"

namespace bm {

void Event::DestroyPacket() {
  CHECK(_event.type == ENET_EVENT_TYPE_RECEIVE);
  enet_packet_destroy(_event.packet);
}

Event::EventType Event::GetType() const {
  if(_event.type == ENET_EVENT_TYPE_CONNECT) {
    return Event::EVENT_CONNECT;
  }
  if(_event.type == ENET_EVENT_TYPE_DISCONNECT) {
    return Event::EVENT_DISCONNECT;
  }
  if(_event.type == ENET_EVENT_TYPE_RECEIVE) {
    return Event::EVENT_RECEIVE;
  }
  return Event::EVENT_NONE;
}

uint8_t Event::GetChannelId() const {
  CHECK(_event.type != ENET_EVENT_TYPE_NONE);
  return _event.channelID;
}

void Event::GetData(std::vector<char>* output) const {
  CHECK(_event.type == ENET_EVENT_TYPE_RECEIVE);
  output->assign(_event.packet->data, _event.packet->data +
    _event.packet->dataLength);
}

Peer* Event::GetPeer() {
  CHECK(_event.type != ENET_EVENT_TYPE_NONE);
  return new Peer(_event.peer);
}

std::string Event::GetPeerIp() const {
  CHECK(_event.type != ENET_EVENT_TYPE_NONE);
  const size_t size = 32;
  char buffer[size];
  if(enet_address_get_host_ip(&_event.peer->address, buffer, size) != 0) {
    bm::Error::Set(bm::Error::TYPE_ENET_GET_HOST_IP);
    buffer[0] = 0;
  }
  return std::string(buffer);
}

uint16_t Event::GetPeerPort() const {
  CHECK(_event.type != ENET_EVENT_TYPE_NONE);
  return _event.peer->address.port;
}

void* Event::GetPeerData() const {
  CHECK(_event.type != ENET_EVENT_TYPE_NONE);
  return _event.peer->data;
}

Event::Event() { }

} // namespace bm
