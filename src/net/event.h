// Copyright (c) 2015 Andrey Konovalov

#ifndef NET_EVENT_H_
#define NET_EVENT_H_

#include <string>
#include <vector>

#include "base/macros.h"
#include "base/pstdint.h"

#include "net/dll.h"

struct _ENetEvent;

namespace bm {

class Enet;
class Host;
class Peer;

// 'Event' class represents an event that can be delivered by
// 'ClientHost::Service()' and 'ServerHost::Service()' methods.
// You can create an empty 'Event' by using 'Enet::CreateEvent()'.
class Event {
  friend class Host;
  friend class Enet;

 public:
  enum EventType {
    // No event occurred within the specified time limit.
    TYPE_NONE,

    // A connection request initiated by 'ClientHost::Connect()' has completed.
    // You can use 'GetPeer()' to get information about the connected peer.
    TYPE_CONNECT,

    // A peer has disconnected. This event is generated on a successful
    // completion of a disconnect initiated by 'Peer::Disconnect()'.
    // You can use 'GetPeer()' to get information about the disconnected peer.
    TYPE_DISCONNECT,

    // A packet has been received from a peer. You can use 'GetPeer()' to get
    // information about peer which sent the packet, 'GetChannelId()' to get
    // the channel number upon which the packet was received, 'GetData()' to
    // get the data from the received packet.
    TYPE_RECEIVE
  };

  BM_NET_DECL ~Event();

  // Returns the type of the event.
  BM_NET_DECL EventType GetType() const;

  // Returns the channel on which the received packet was sent.
  // Event type should not be 'TYPE_NONE' to use this method.
  BM_NET_DECL uint8_t GetChannelId() const;

  // Returns the data associated with the event. Data will be lost if
  // another event is delivered using 'ClientHost::Service()' or
  // 'ServerHost::Service()' using this instance of 'Event' class.
  // Event type should be 'TYPE_RECEIVE' to use this method.
  BM_NET_DECL void GetData(std::vector<char>* output) const;

  // Returns 'Peer', which caused the event. Returned 'Peer' will be
  // deallocated automatically.
  // Event type should not be 'TYPE_NONE' to use this method.
  BM_NET_DECL Peer* GetPeer();

 private:
  // Creates an uninitialized 'Event'. Don't use it yourself.
  // You can create an 'Event' by using 'Enet::CreateEvent()'.
  Event();

  // Destroys the packet that is hold by '_event'. If it hasn't been
  // received yet or has been destroyed already - nothing happens.
  void _DestroyPacket();

  _ENetEvent* _event;

  // 'False' if a packet received using 'ClientHost::Service()' or
  // 'ServerHost::Service()' hasn't been deallocated yet.
  bool _is_packet_destroyed;

  // 'Host' that generated the event.
  Host* _host;

  DISALLOW_COPY_AND_ASSIGN(Event);
};

}  // namespace bm

#endif  // NET_EVENT_H_
