#ifndef BLOWMORPH_CLIENT_GAME_CONTROLLER_HPP_
#define BLOWMORPH_CLIENT_GAME_CONTROLLER_HPP_

#include <base/macros.hpp>
#include <base/protocol.hpp> // XXX: do we need it here?

namespace bm {

class PacketProcesser;
class Window;

class GameController {
public:
  GameController();
  ~GameController();

  bool Initialize(PacketProcesser* packet_processer, Window* window);
  bool Finalize();

  bool Update();

  uint32_t GetTime();
  uint32_t GetCorrectedTime();

  void ProcessClientOptions(const ClientOptions& client_options);
  void ProcessTimeSyncData(const TimeSyncData& time_dync_data);

private:
  DISALLOW_COPY_AND_ASSIGN(GameController);

  PacketProcesser* _packet_processer;
  Window* _window;

  ClientOptions _client_options;
  TimeSyncData _time_sync_data;

  uint32_t _time_correction;

  enum {
    STATE_GAME_STARTED,
    STATE_GAME_TIME_SYNC_READY,
    STATE_GAME_TIME_SYNC_REQUEST_SENT,
    STATE_GAME_LOGGED_IN
  } _game_state;

  enum {
    STATE_FINALIZED,
    STATE_INITIALIZED
  } _state;
};

} // namespace bm

#endif // BLOWMORPH_CLIENT_GAME_CONTROLLER_HPP_
