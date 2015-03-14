// Copyright (c) 2013 Blowmorph Team

#ifndef BASE_NET_H_
#define BASE_NET_H_

#include <vector>

#include <cstring>

#include <enet-plus/enet.h>

#include "base/error.h"
#include "base/macros.h"
#include "base/pstdint.h"

namespace bm {

// Returns 'false' when packet format is incorrect.
template<class PacketType>
bool ExtractPacketType(const std::vector<char>& buffer, PacketType* type) {
  CHECK(type != NULL);
  if (buffer.size() < sizeof(*type)) {
    return false;
  }
  memcpy(type, &buffer[0], sizeof(*type));
  return true;
}

// Returns 'false' when message format is incorrect.
template<class PacketType, class DataType>
bool ExtractPacketData(const std::vector<char>& buffer, DataType* data) {
  CHECK(data != NULL);
  if (buffer.size() != sizeof(PacketType) + sizeof(DataType)) {
    return false;
  }
  memcpy(data, &buffer[0] + sizeof(PacketType), sizeof(DataType));
  return true;
}

// Appends packet type and data to the end of the buffer.
template<class PacketType, class DataType>
void AppendPacketToBuffer(
  std::vector<char>& buffer,
  PacketType packet_type,
  const DataType& data
) {
  buffer.insert(buffer.end(),
    reinterpret_cast<const char*>(&packet_type),
    reinterpret_cast<const char*>(&packet_type) + sizeof(packet_type));

  buffer.insert(buffer.end(),
    reinterpret_cast<const char*>(&data),
    reinterpret_cast<const char*>(&data) + sizeof(data));
}

template<class PacketType, class DataType>
bool SendPacket(
    enet::Peer* peer,
    PacketType packet_type,
    const DataType& data,
    bool reliable = false
) {
  std::vector<char> buffer;
  AppendPacketToBuffer(buffer, packet_type, data);

  bool rv = peer->Send(&buffer[0], buffer.size(), reliable);
  if (rv == false) {
    REPORT_ERROR("Couldn't send packet.");
    return false;
  }
  return true;
}

template<class PacketType, class DataType>
bool BroadcastPacket(
    enet::ServerHost* host,
    PacketType packet_type,
    const DataType& data,
    bool reliable = false
) {
  std::vector<char> buffer;
  AppendPacketToBuffer(buffer, packet_type, data);

  bool rv = host->Broadcast(&buffer[0], buffer.size(), reliable);
  if (rv == false) {
    REPORT_ERROR("Couldn't broadcast packet.");
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
