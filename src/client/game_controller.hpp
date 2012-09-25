#ifndef BLOWMORPH_CLIENT_GAME_CONTROLLER_HPP_
#define BLOWMORPH_CLIENT_GAME_CONTROLLER_HPP_

namespace bm {

class PacketProcesser;

class GameController {
public:
  GameController();
  ~GameController();

  bool Initialize(PacketProcesser* packet_processer);
  bool Finalize();

private:
  PacketProcesser* _packet_processer;

  enum {
    STATE_FINALIZED,
    STATE_INITIALIZED
  } _state;
};

} // namespace bm

#endif // BLOWMORPH_CLIENT_GAME_CONTROLLER_HPP_
