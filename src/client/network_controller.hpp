#ifndef BLOWMORPH_CLIENT_NETWORK_CONTROLLER_HPP_
#define BLOWMORPH_CLIENT_NETWORK_CONTROLLER_HPP_

#include <cstdlib>

#include <list>
#include <string>

#include <enet-wrapper/enet.hpp>

#include <base/macros.hpp>
#include <base/pstdint.hpp>

namespace bm {

class NetworkController {
public:
  class Listener {
  public:
    Listener() { };
    ~Listener() { };

    virtual void OnConnect() = 0;
    virtual void OnDisconnect() = 0;

    virtual void OnMessageSent(const char* message, size_t length) = 0;
    virtual void OnMessageReceived(const char* message, size_t length) = 0;
  };

public:
  NetworkController();
  ~NetworkController();

  bool Initialize(const std::string& host, int port);
  void Finalize();

  bool Service(/*uint32_t timeout*/);

  bool Connect(uint32_t timeout);
  bool Disconnect(uint32_t timeout);

  bool SendMessage(const char* message, size_t length);

  bool RegisterListener(Listener* listener);
  bool UnregisterListener(Listener* listener);

private:
  DISALLOW_COPY_AND_ASSIGN(NetworkController);

  std::string _host;
  int _port;

  Enet _enet;
  ClientHost* _client;
  Peer* _peer;
  Event* _event;

  std::list<Listener*> _listeners;

  enum {
    NETWORK_STATE_DISCONNECTED,
    NETWORK_STATE_CONNECTED
  } _network_state;

  enum {
    STATE_FINALIZED,
    STATE_INITIALIZED
  } _state;
};

} // namespace bm

#endif // BLOWMORPH_CLIENT_NETWORK_CONTROLLER_HPP_
