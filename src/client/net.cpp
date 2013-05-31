#include "net.hpp"
#include "sys.hpp"

namespace bm { namespace net {

// Sends input events to the server and
// clears the input event queues afterwards.
bool SendInputEvents(enet::Peer* peer, std::vector<bm::KeyboardEvent>& keyboard_events, std::vector<bm::MouseEvent>& mouse_events) {
  std::vector<char> buf;

  for(size_t i = 0; i < keyboard_events.size(); i++) {
    buf.clear();

    AppendPacketToBuffer(buf, &keyboard_events[i], bm::Packet::TYPE_KEYBOARD_EVENT);

    bool rv = peer->Send(&buf[0], buf.size());
    if(rv == false) {
      return false;
    }
  }
  keyboard_events.clear();

  for(size_t i = 0; i < mouse_events.size(); i++) {
    buf.clear();

    AppendPacketToBuffer(buf, &mouse_events[i], bm::Packet::TYPE_MOUSE_EVENT);

    bool rv = peer->Send(&buf[0], buf.size());
    if(rv == false) {
      return false;
    }
  }
  mouse_events.clear();

  return true;
}

// Attempts to synchronously disconnect the peer.
bool DisconnectPeer(enet::Peer* peer, enet::Event* event, enet::ClientHost* host, uint32_t timeout) {
  CHECK(peer != NULL);
  CHECK(event != NULL);
  CHECK(host != NULL);

  peer->Disconnect();

  double start = sys::Timestamp();

  while(sys::Timestamp() - start <= timeout / 1000.0f) {
    bool rv = host->Service(event, (uint32_t) timeout);
    if(rv == false) {
      return false;
    }

    if(event != NULL && event->GetType() == enet::Event::TYPE_DISCONNECT) {
      return true;
    }
  }

  return false;
}

}}