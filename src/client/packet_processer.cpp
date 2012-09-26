#include "packet_processer.hpp"

#include <cstdlib>

#include <base/error.hpp>
#include <base/macros.hpp>

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
  if(_network_controller_listener == NULL) {
    BM_ERROR("Could not allocate memory!");
    return false;
  }

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
  _packet_processer->_Decode(message, length);
}

#include <cstdio>

bool PacketProcesser::_Decode(const char* message, size_t length) {
  // TODO.
  fprintf(stderr, "Received %u bytes: '", length);
  for(size_t i = 0; i < length; i++) {
    fprintf(stderr, "%c", message[i]);
  }
  fprintf(stderr, "'\n");
  return true;
}

NetworkController* _network_controller;
GameController* _game_controller;

enum {
  STATE_FINALIZED,
  STATE_INITIALIZED
} _state;

} // namespace bm
