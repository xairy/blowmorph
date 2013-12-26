// Copyright (c) 2013 Blowmorph Team

#ifndef BASE_NET_H_
#define BASE_NET_H_

#include <vector>

#include <cstring>

#include <enet-plus/enet.h>

#include "base/error.h"
#include "base/macros.h"
#include "base/protocol.h"
#include "base/pstdint.h"

namespace bm {

// Returns 'false' when packet format is incorrect.
bool ExtractPacketType(const std::vector<char>& buffer, Packet::Type* type) {
  CHECK(type != NULL);
  if (buffer.size() < sizeof(Packet::Type)) {
    return false;
  }
  memcpy(type, &buffer[0], sizeof(Packet::Type));
  return true;
}

// Returns 'false' when message format is incorrect.
template<class T>
bool ExtractPacketData(const std::vector<char>& buffer, T* data) {
  CHECK(data != NULL);
  if (buffer.size() != sizeof(Packet::Type) + sizeof(T)) {
    return false;
  }
  memcpy(data, &buffer[0] + sizeof(Packet::Type), sizeof(T));
  return true;
}

// Appends packet type and data to the end of the buffer.
template<class T> void AppendPacketToBuffer(
  std::vector<char>& buffer,
  Packet::Type packet_type,
  const T& data
) {
  CHECK(Packet::TYPE_UNKNOWN < packet_type);
  CHECK(packet_type < Packet::TYPE_MAX_VALUE);

  buffer.insert(buffer.end(),
    reinterpret_cast<const char*>(&packet_type),
    reinterpret_cast<const char*>(&packet_type) + sizeof(packet_type));

  buffer.insert(buffer.end(),
    reinterpret_cast<const char*>(&data),
    reinterpret_cast<const char*>(&data) + sizeof(data));
}

template<class T> bool SendPacket(
    enet::Peer* peer,
    Packet::Type packet_type,
    const T& data,
    bool reliable = false
) {
  std::vector<char> buffer;
  AppendPacketToBuffer(buffer, packet_type, data);

  bool rv = peer->Send(&buffer[0], buffer.size(), reliable);
  if (rv == false) {
    THROW_ERROR("Couldn't send packet.");
    return false;
  }
  return true;
}

template<class T> bool BroadcastPacket(
    enet::ServerHost* host,
    Packet::Type packet_type,
    const T& data,
    bool reliable = false
) {
  std::vector<char> buffer;
  AppendPacketToBuffer(buffer, packet_type, data);

  bool rv = host->Broadcast(&buffer[0], buffer.size(), reliable);
  if (rv == false) {
    THROW_ERROR("Couldn't broadcast packet.");
    return false;
  }
  return true;
}

// Attempts to synchronously disconnect the peer.
bool DisconnectPeer(
  enet::Peer* peer,
  enet::Event* event,
  enet::ClientHost* host,
  uint32_t timeout
);

}  // namespace bm

#endif  // BASE_NET_H_
