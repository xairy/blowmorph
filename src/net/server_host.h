// Copyright (c) 2015 Andrey Konovalov

#ifndef NET_SERVER_HOST_H_
#define NET_SERVER_HOST_H_

#include "base/macros.h"
#include "base/pstdint.h"

#include "net/host.h"

#include "net/dll.h"

struct _ENetHost;

namespace bm {

class Enet;
class Event;

// A server host for communicating with client hosts.
// You can create a 'ServerHost' using 'Enet::CreateServerHost'.
class ServerHost : public Host {
  friend class Enet;

 public:
  BM_NET_DECL ~ServerHost();

  // Initializes 'ServerHost'. 'ServerHost' starts on port 'port'.
  // You may specify 'peer_count' - the maximum allowable number of
  // connected peers, 'channel_count' - the number of channels to be used.
  // You may specify incoming and outgoing bandwidth of the server in bytes
  // per second. Specifying '0' for these two options will cause ENet to rely
  // entirely upon its dynamic throttling algorithm to manage bandwidth.
  BM_NET_DECL bool Initialize(
    uint16_t port,
    size_t peer_count = 32,
    size_t channel_count = 1,
    uint32_t incoming_bandwidth = 0,
    uint32_t outgoing_bandwidth = 0);

  // Cleans up. Automatically called in the destructor.
  BM_NET_DECL void Finalize();

  // Broadcast data from 'data' with length of 'length' to all Peer's on
  // selected channel, associated with 'channel_id'.
  // Returns 'true' on success, returns 'false' on error.
  BM_NET_DECL bool Broadcast(
    const char* data,
    size_t length,
    bool reliable = true,
    uint8_t channel_id = 0);

  // Look in 'host.hpp' for the description.
  // BM_NET_DECL virtual bool Service(Event* event, uint32_t timeout);

  // Look in 'host.hpp' for the description.
  // BM_NET_DECL virtual void Flush();

 private:
  // Creates an uninitialized 'ServerHost'.
  ServerHost();

  enum {
    STATE_INITIALIZED,
    STATE_FINALIZED
  } _state;

  DISALLOW_COPY_AND_ASSIGN(ServerHost);
};

}  // namespace bm

#endif  // NET_SERVER_HOST_H_
