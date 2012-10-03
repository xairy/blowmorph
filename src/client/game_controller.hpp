#ifndef BLOWMORPH_CLIENT_GAME_CONTROLLER_HPP_
#define BLOWMORPH_CLIENT_GAME_CONTROLLER_HPP_

#include <base/macros.hpp>

namespace bm {

class PacketProcesser;

class GameController {
public:
  GameController();
  ~GameController();

  bool Initialize(PacketProcesser* packet_processer);
  bool Finalize();

private:
  DISALLOW_COPY_AND_ASSIGN(GameController);

  PacketProcesser* _packet_processer;

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
