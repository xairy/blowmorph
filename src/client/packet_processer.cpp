#include "packet_processer.hpp"

#include <cstdio>

#include <base/error.hpp>
#include <base/macros.hpp>
#include <base/protocol.hpp>

#include "game_controller.hpp"
#include "network_controller.hpp"

namespace bm {

PacketProcesser::PacketProcesser() : _state(STATE_FINALIZED) { }
PacketProcesser::~PacketProcesser() {
  CHECK(_state == STATE_FINALIZED);
}

bool PacketProcesser::Initialize(
  NetworkController* network_controller,
  GameController* game_controller
) {
  CHECK(_state == STATE_FINALIZED);
  CHECK(network_controller != NULL);
  CHECK(game_controller != NULL);

  _network_controller = network_controller;
  _game_controller = game_controller;

  _network_controller_listener = new NetworkControllerListener(this);
  CHECK(_network_controller_listener != NULL);

  bool rv = _network_controller->RegisterListener(_network_controller_listener);
  if(rv == false) {
    BM_ERROR("Could not register network controller listener");
    return false;
  }

  _state = STATE_INITIALIZED;
  return true;
}

bool PacketProcesser::Finalize() {
  CHECK(_state == STATE_INITIALIZED);
  bool rv = _network_controller->UnregisterListener(_network_controller_listener);
  if(rv == false) {
    BM_ERROR("Could not unregister network controller listener");
    return false;
  }
  _state = STATE_FINALIZED;
  return true;
}

PacketProcesser::NetworkControllerListener::NetworkControllerListener(PacketProcesser* packet_processer) {
  CHECK(packet_processer != NULL);
  _packet_processer = packet_processer;
}
PacketProcesser::NetworkControllerListener::~NetworkControllerListener() { }

void PacketProcesser::NetworkControllerListener::OnConnect() { }
void PacketProcesser::NetworkControllerListener::OnDisconnect() { }
void PacketProcesser::NetworkControllerListener::OnMessageSent(const char* message, size_t length) { }
void PacketProcesser::NetworkControllerListener::OnMessageReceived(const char* message, size_t length) {
  _packet_processer->decode(message, length);
}

bool PacketProcesser::SendTimeSyncRequest(uint32_t time) {
  TimeSyncData request_data;
  request_data.client_time = time;
  Packet::Type request_type = Packet::TYPE_SYNC_TIME_REQUEST;

  std::vector<char> message;
  message.insert(message.end(), reinterpret_cast<char*>(&request_type),
    reinterpret_cast<char*>(&request_type) + sizeof(request_type));
  message.insert(message.end(), reinterpret_cast<char*>(&request_data),
    reinterpret_cast<char*>(&request_data) + sizeof(request_data));

  bool rv = _network_controller->SendMessage(&message[0], message.size());
  if(rv == false) {
    return false;
  }
  return true;
}

#include <cstdio>

bool PacketProcesser::decode(const char* message, size_t length) {
  if(length < sizeof(Packet::Type)) {
    BM_ERROR("Packet has wrong format, it is too small.");
    return false;
  }

  const Packet::Type* packet_type = reinterpret_cast<const Packet::Type*>(message);
  if(*packet_type == Packet::TYPE_CLIENT_OPTIONS) {
    bool rv = decodeClientOptions(message + sizeof(Packet::Type), length - sizeof(Packet::Type));
    if(rv == false) {
      BM_ERROR("Could not decode client options packet!");
      return false;
    }
    printf("Received client options.\n");
    return true;
  } else if(*packet_type == Packet::TYPE_SYNC_TIME_RESPONSE) {
    bool rv = decodeTimeSyncData(message + sizeof(Packet::Type), length - sizeof(Packet::Type));
    if(rv == false) {
      BM_ERROR("Could not decode time synchronization data packet!");
      return false;
    }
    printf("Received time synchronization data.\n");
    return true;
  }

  /*fprintf(stderr, "Received %u bytes: '", length);
  for(size_t i = 0; i < length; i++) {
    fprintf(stderr, "%c", message[i]);
  }
  fprintf(stderr, "'\n");*/
  return true;
}

bool PacketProcesser::decodeClientOptions(const char* data, size_t length) {
  if(length != sizeof(ClientOptions)) {
    BM_ERROR("Wrong client options packet size!");
    return false;
  }
  ClientOptions client_options = *reinterpret_cast<const ClientOptions*>(data);
  _game_controller->ProcessClientOptions(client_options);
  return true;
}

bool PacketProcesser::decodeTimeSyncData(const char* data, size_t length) {
  if(length != sizeof(TimeSyncData)) {
    BM_ERROR("Wrong client options packet size!");
    return false;
  }
  TimeSyncData time_sync_data = *reinterpret_cast<const TimeSyncData*>(data);
  _game_controller->ProcessTimeSyncData(time_sync_data);
  return true;
}

} // namespace bm
