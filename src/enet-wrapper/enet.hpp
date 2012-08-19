#ifndef BLOWMORPH_ENET_WRAPPER_ENET_HPP_
#define BLOWMORPH_ENET_WRAPPER_ENET_HPP_

#include "base/macros.hpp"
#include "base/pstdint.hpp"

#include "enet-wrapper/server_host.hpp"
#include "enet-wrapper/client_host.hpp"
#include "enet-wrapper/event.hpp"
#include "enet-wrapper/peer.hpp"

#include "library.hpp"

namespace bm {

// TODO: write comments.
class Enet {
public:
  // TODO: write comments.
  BM_ENET_DECL Enet();

  // TODO: write comments.
  BM_ENET_DECL ~Enet();

  // Initializes ENet.
  // Returns 'true' on success, returns 'false' on error.
  BM_ENET_DECL bool Initialize();

  // Creates 'ServerHost' bound to 'port'.
  // You may specify 'channel_count' - number of channels to be used.
  // You may specify incoming and outgoing bandwidth of the server in bytes
  // per second. Specifying '0' for these two options will cause ENet to rely
  // entirely upon its dynamic throttling algorithm to manage bandwidth.
  // Returned 'ServerHost' should be deallocated manually using 'delete'.
  BM_ENET_DECL ServerHost* CreateServerHost(
    uint16_t port,
    size_t peer_count = 32,
    size_t channel_count = 1,
    uint32_t incoming_bandwith = 0,
    uint32_t outgoing_bandwith = 0
  );

  // Creates 'ClientHost'.
  // You may specify 'channel_count' - number of channels to be used.
  // You may specify incoming and outgoing bandwidth of the server in bytes
  // per second. Specifying '0' for these two options will cause ENet to rely
  // entirely upon its dynamic throttling algorithm to manage bandwidth.
  // Returned 'ClientHost' should be deallocated manually using 'delete'.
  BM_ENET_DECL ClientHost* CreateClientHost(
    size_t channel_count = 1,
    uint32_t incoming_bandwith = 0,
    uint32_t outgoing_bandwith = 0
  );

  // Creates non-initialized Event.
  // Returned 'Event' should be deallocated manually using 'delete'.
  BM_ENET_DECL Event* CreateEvent();

private:
  DISALLOW_COPY_AND_ASSIGN(Enet);

  enum {
    STATE_CREATED,
    STATE_INITIALIZED,
  } _state;
};

} // namespace bm

#endif // BLOWMORPH_ENET_WRAPPER_ENET_HPP_
