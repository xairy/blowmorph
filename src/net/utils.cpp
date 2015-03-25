// Copyright (c) 2015 Blowmorph Team

#include "net/utils.h"

#include <vector>

#include "base/time.h"

#include "net/enet.h"

namespace bm {

// Attempts to synchronously disconnect the peer.
bool DisconnectPeer(
  Peer* peer,
  Event* event,
  ClientHost* host,
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

    if (event != NULL && event->GetType() == Event::TYPE_DISCONNECT) {
      return true;
    }
  }

  return false;
}

}  // namespace bm
