#ifndef BLOWMORPH_CLIENT_PACKET_PROCESSER_HPP_
#define BLOWMORPH_CLIENT_PACKET_PROCESSER_HPP_

#include <cstdlib>

#include <base/macros.hpp>

#include "network_controller.hpp"

namespace bm {

class GameController;

class PacketProcesser {
public:
  PacketProcesser();
  ~PacketProcesser();

  bool Initialize(NetworkController* network_controller, GameController* game_controller);
  bool Finalize();

private:
  bool decode(const char* message, size_t length);

  bool decodeClientOptions(const char* data, size_t length);

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
    DISALLOW_COPY_AND_ASSIGN(NetworkControllerListener);

    PacketProcesser* _packet_processer;
  };

  friend class NetworkControllerListener;

private:
  DISALLOW_COPY_AND_ASSIGN(PacketProcesser);

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
