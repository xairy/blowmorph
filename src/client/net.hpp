#ifndef BLOWMORPH_CLIENT_NET_HPP_
#define BLOWMORPH_CLIENT_NET_HPP_

#include <vector>

#include <base/error.hpp>
#include <base/macros.hpp>
#include <base/protocol.hpp>
#include <base/pstdint.hpp>

#include <enet-plus/enet.hpp>

namespace bm { namespace net {

// Appends packet type and data to the end of the buffer.
template<class T> void AppendPacketToBuffer(std::vector<char>& buf, const T* data, bm::Packet::Type packet_type) {
  CHECK(data != NULL);
  CHECK(bm::Packet::TYPE_UNKNOWN <= packet_type && packet_type <= bm::Packet::TYPE_MAX_VALUE);

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
bool SendInputEvents(enet::Peer* peer, std::vector<bm::KeyboardEvent>& keyboard_events, std::vector<bm::MouseEvent>& mouse_events);

// Attempts to synchronously disconnect the peer.
bool DisconnectPeer(enet::Peer* peer, enet::Event* event, enet::ClientHost* host, uint32_t timeout);

} }

#endif  // BLOWMORPH_CLIENT_NET_HPP_
