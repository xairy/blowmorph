// Copyright (c) 2013 Blowmorph Team

#include "base/net.h"

#include <vector>

#include <enet-plus/enet.h>

#include "base/time.h"

namespace bm {

// Attempts to synchronously disconnect the peer.
bool DisconnectPeer(
  enet::Peer* peer,
  enet::Event* event,
  enet::ClientHost* host,
  uint32_t timeout
) {
  CHECK(peer != NULL);
  CHECK(event != NULL);
  CHECK(host != NULL);

  peer->Disconnect();

  int64_t start = Timestamp();

  while (Timestamp() - start <= timeout) {
    bool rv = host->Service(event, (uint32_t) timeout);
    if (rv == false) {
      return false;
    }

    if (event != NULL && event->GetType() == enet::Event::TYPE_DISCONNECT) {
      return true;
    }
  }

  return false;
}

}  // namespace bm
