// Copyright (c) 2015 Andrey Konovalov

#ifndef NET_ENET_H_
#define NET_ENET_H_

#include "base/macros.h"
#include "base/pstdint.h"

#include "net/host.h"
#include "net/server_host.h"
#include "net/client_host.h"
#include "net/event.h"
#include "net/peer.h"

#include "net/dll.h"

namespace bm {

// You need to create and 'Initialize()' a 'Enet' instance to work with ENet.
class Enet {
 public:
  BM_NET_DECL Enet();
  BM_NET_DECL ~Enet();

  // Initializes ENet.
  // Returns 'true' on success, returns 'false' on error.
  BM_NET_DECL bool Initialize();

  // Cleans up.
  // Automatically called in the destructor.
  BM_NET_DECL void Finalize();

  // Creates 'ServerHost' bound to 'port'.
  // You may specify 'channel_count' - number of channels to be used.
  // You may specify incoming and outgoing bandwidth of the server in bytes
  // per second. Specifying '0' for these two options will cause ENet to rely
  // entirely upon its dynamic throttling algorithm to manage bandwidth.
  // Returned 'ServerHost' should be deallocated manually using 'delete'.
  // Returns 'NULL' on error.
  BM_NET_DECL ServerHost* CreateServerHost(
    uint16_t port,
    size_t peer_count = 32,
    size_t channel_count = 1,
    uint32_t incoming_bandwith = 0,
    uint32_t outgoing_bandwith = 0);

  // Creates 'ClientHost'.
  // You may specify 'channel_count' - number of channels to be used.
  // You may specify incoming and outgoing bandwidth of the server in bytes
  // per second. Specifying '0' for these two options will cause ENet to rely
  // entirely upon its dynamic throttling algorithm to manage bandwidth.
  // Returned 'ClientHost' should be deallocated manually using 'delete'.
  // Returns 'NULL' on error.
  BM_NET_DECL ClientHost* CreateClientHost(
    size_t channel_count = 1,
    uint32_t incoming_bandwith = 0,
    uint32_t outgoing_bandwith = 0);

  // Creates an empty Event.
  // Returned 'Event' should be deallocated manually using 'delete'.
  BM_NET_DECL Event* CreateEvent();

 private:
  enum {
    STATE_FINALIZED,
    STATE_INITIALIZED,
  } _state;

  DISALLOW_COPY_AND_ASSIGN(Enet);
};

}  // namespace bm

#endif  // NET_ENET_H_
