// Copyright (c) 2015 Andrey Konovalov

#ifndef NET_CLIENT_HOST_H_
#define NET_CLIENT_HOST_H_

#include <string>

#include "base/macros.h"
#include "base/pstdint.h"

#include "net/host.h"

#include "net/dll.h"

struct _ENetHost;

namespace bm {

class Enet;
class Event;
class Peer;

// A client host for communicating with a server host.
// You can create a 'ClientHost' using 'Enet::CreateClientHost'.
class ClientHost : public Host {
  friend class Enet;

 public:
  BM_NET_DECL ~ClientHost();

  // Initializes 'ClientHost'.
  // You may specify 'channel_count' - number of channels to be used.
  // You may specify incoming and outgoing bandwidth of the server in bytes
  // per second. Specifying '0' for these two options will cause ENet to rely
  // entirely upon its dynamic throttling algorithm to manage bandwidth.
  BM_NET_DECL bool Initialize(
    size_t channel_count = 1,
    uint32_t incoming_bandwidth = 0,
    uint32_t outgoing_bandwidth = 0);

  // Cleans up. Automatically called in the destructor.
  BM_NET_DECL void Finalize();

  // Initiates connection procedure to another host. To complete connection, an
  // event 'EVENT_CONNECT' should be dispatched using 'ClientHost::Service()'.
  // You may specify 'channel_count' - number of channels to be used.
  // Returns 'Peer' on success, returns 'NULL' on error.
  // Returned 'Peer' will be deallocated automatically.
  BM_NET_DECL Peer* Connect(
    std::string server_ip,
    uint16_t port,
    size_t channel_count = 1);

  // Look in 'host.hpp' for the description.
  // BM_NET_DECL virtual bool Service(Event* event, uint32_t timeout);

  // Look in 'host.hpp' for the description.
  // BM_NET_DECL virtual void Flush();

 private:
  // Creates an uninitialized 'ClientHost'.
  ClientHost();

  enum {
    STATE_FINALIZED,
    STATE_INITIALIZED,
  } _state;

  DISALLOW_COPY_AND_ASSIGN(ClientHost);
};

}  // namespace bm

#endif  // NET_CLIENT_HOST_H_
