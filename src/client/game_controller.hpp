#ifndef BLOWMORPH_CLIENT_GAME_CONTROLLER_HPP_
#define BLOWMORPH_CLIENT_GAME_CONTROLLER_HPP_

#include <base/macros.hpp>
#include <base/protocol.hpp> // XXX: do we need it here?

namespace bm {

class PacketProcesser;

class GameController {
public:
  GameController();
  ~GameController();

  bool Initialize(PacketProcesser* packet_processer);
  bool Finalize();

  void SetClientOptions(const ClientOptions& client_options);

private:
  DISALLOW_COPY_AND_ASSIGN(GameController);

  PacketProcesser* _packet_processer;

  ClientOptions _client_options;

  enum {
    STATE_GAME_STARTED,
    STATE_GAME_SYNCHRONIZATION,
    STATE_GAME_LOGGED_IN
  } _game_state;

  enum {
    STATE_FINALIZED,
    STATE_INITIALIZED
  } _state;
};

} // namespace bm

#endif // BLOWMORPH_CLIENT_GAME_CONTROLLER_HPP_
