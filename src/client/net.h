// Copyright (c) 2013 Blowmorph Team

#ifndef SRC_CLIENT_NET_H_
#define SRC_CLIENT_NET_H_

#include <vector>

#include <enet-plus/enet.hpp>

#include "base/error.h"
#include "base/macros.h"
#include "base/protocol.h"
#include "base/pstdint.h"

namespace bm {
namespace net {

// Appends packet type and data to the end of the buffer.
template<class T> void AppendPacketToBuffer(
  std::vector<char>& buf,
  const T* data,
  bm::Packet::Type packet_type
) {
  CHECK(data != NULL);
  CHECK(bm::Packet::TYPE_UNKNOWN <= packet_type &&
        packet_type <= bm::Packet::TYPE_MAX_VALUE);

  // Append packet type.
  buf.insert(buf.end(),
    reinterpret_cast<const char*>(&packet_type),
    reinterpret_cast<const char*>(&packet_type) + sizeof(packet_type));

  // Append packet data.
  buf.insert(buf.end(),
    reinterpret_cast<const char*>(data),
    reinterpret_cast<const char*>(data) + sizeof(*data));
}

// Sends input events to the server and
// clears the input event queues afterwards.
bool SendInputEvents(
  enet::Peer* peer,
  std::vector<bm::KeyboardEvent>& keyboard_events,
  std::vector<bm::MouseEvent>& mouse_events
);

// Attempts to synchronously disconnect the peer.
bool DisconnectPeer(
  enet::Peer* peer,
  enet::Event* event,
  enet::ClientHost* host,
  uint32_t timeout
);

}  // namespace net
}  // namespace bm

#endif  // SRC_CLIENT_NET_H_
