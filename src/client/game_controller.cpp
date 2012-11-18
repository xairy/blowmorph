#include "game_controller.hpp"

#include <cstdio>

#include <base/error.hpp>
#include <base/macros.hpp>
#include <base/protocol.hpp>

#include "packet_processer.hpp"
#include "window.hpp"

namespace bm {

GameController::GameController() : _state(STATE_FINALIZED) { }
GameController::~GameController() {
  CHECK(_state == STATE_FINALIZED);
}

bool GameController::Initialize(PacketProcesser* packet_processer, Window* window) {
  CHECK(_state == STATE_FINALIZED);

  _packet_processer = packet_processer;
  _window = window;
  _game_state = STATE_GAME_STARTED;
  _time_correction = TimeType(0);

  _state = STATE_INITIALIZED;
  return true;
}

bool GameController::Finalize() {
  CHECK(_state == STATE_INITIALIZED);

  // TODO.

  _state = STATE_FINALIZED;
  return true;
}

bool GameController::Update() {
  CHECK(_state == STATE_INITIALIZED);

  if(_game_state == STATE_GAME_TIME_SYNC_READY) {
    bool rv = _packet_processer->SendTimeSyncRequest(GetTime());
    if(rv == false) {
      BM_ERROR("Could not send time synchronization request!");
      return false;
    }
    _game_state = STATE_GAME_TIME_SYNC_REQUEST_SENT;
  }

  return true;
}

TimeType GameController::GetTime() {
  // TODO.
  return TimeType(0);
}

TimeType GameController::GetCorrectedTime() {
  return GetTime() + _time_correction;
}

void GameController::ProcessClientOptions(const ClientOptions& client_options) {
  CHECK(_state == STATE_INITIALIZED);
  CHECK(_game_state == STATE_GAME_STARTED);

  _client_options = client_options;

  _game_state = STATE_GAME_TIME_SYNC_READY;
}

void GameController::ProcessTimeSyncData(const TimeSyncData& time_sync_data) {
  CHECK(_state == STATE_INITIALIZED);
  CHECK(_game_state == STATE_GAME_TIME_SYNC_REQUEST_SENT);

  _time_sync_data = time_sync_data;

  TimeType time = GetTime();

  // XXX[xairy]: remove 'CHECK's when all time variables are 'int64_t'?
  CHECK(time >= time_sync_data.client_time);
  TimeType latency = time - time_sync_data.client_time;

  CHECK(time_sync_data.server_time + latency / 2 >= time);
  _time_correction = time_sync_data.server_time + latency / 2 - time;

  printf("Time correction: %u\n", _time_correction);

  _game_state = STATE_GAME_LOGGED_IN;
}

} // namespace bm
