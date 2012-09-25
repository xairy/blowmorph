#ifndef BLOWMORPH_CLIENT_PACKET_PROCESSER_HPP_
#define BLOWMORPH_CLIENT_PACKET_PROCESSER_HPP_

#include <cstdlib>

namespace bm {

class NetworkController;
class GameController;

class PacketProcesser {
public:
  PacketProcesser();
  ~PacketProcesser();

  bool Initialize(NetworkController* network_controller, GameController* game_controller);
  bool Finalize();

  bool Decode(const char* message, size_t length);

private:
  NetworkController* _network_controller;
  GameController* _game_controller;

  enum {
    STATE_FINALIZED,
    STATE_INITIALIZED
  } _state;
};

} // namespace bm

#endif // BLOWMORPH_CLIENT_PACKET_PROCESSER_HPP_
