#ifndef BLOWMORPH_CLIENT_PACKET_PROCESSER_HPP_
#define BLOWMORPH_CLIENT_PACKET_PROCESSER_HPP_

#include <cstdlib>

#include "network_controller.hpp"

namespace bm {

//class NetworkController;
//class NetworkController::Listener;
class GameController;

class PacketProcesser {
public:
  PacketProcesser();
  ~PacketProcesser();

  bool Initialize(NetworkController* network_controller, GameController* game_controller);
  bool Finalize();

private:
  bool _Decode(const char* message, size_t length);

private:
  class NetworkControllerListener : public NetworkController::Listener {
  public:
    NetworkControllerListener(PacketProcesser* packet_processer);
    ~NetworkControllerListener();

    virtual void OnConnect();
    virtual void OnDisconnect();

    virtual void OnMessageSent(const char* message, size_t length);
    virtual void OnMessageReceived(const char* message, size_t length);

  private:
    PacketProcesser* _packet_processer;
  };

  friend class NetworkControllerListener;

private:
  NetworkController* _network_controller;
  GameController* _game_controller;

  NetworkControllerListener* _network_controller_listener;

  enum {
    STATE_FINALIZED,
    STATE_INITIALIZED
  } _state;
};

} // namespace bm

#endif // BLOWMORPH_CLIENT_PACKET_PROCESSER_HPP_
