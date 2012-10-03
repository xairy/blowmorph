#include "game_controller.hpp"

#include <base/macros.hpp>
#include <base/protocol.hpp>

namespace bm {

GameController::GameController() : _state(STATE_FINALIZED) { }
GameController::~GameController() {
  CHECK(_state == STATE_FINALIZED);
}

bool GameController::Initialize(PacketProcesser* packet_processer) {
  CHECK(_state == STATE_FINALIZED);

  _packet_processer = packet_processer;
  _game_state = STATE_GAME_STARTED;

  _state = STATE_INITIALIZED;
  return true;
}

bool GameController::Finalize() {
  CHECK(_state == STATE_INITIALIZED);

  // TODO.

  _state = STATE_FINALIZED;
  return true;
}

void GameController::SetClientOptions(const ClientOptions& client_options) {
  CHECK(_state == STATE_INITIALIZED);
  CHECK(_game_state == STATE_GAME_STARTED);

  _client_options = client_options;

  _game_state = STATE_GAME_SYNCHRONIZATION;
}

} // namespace bm
