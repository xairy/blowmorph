#ifndef BLOWMORPH_CLIENT_NETWORK_CONTROLLER_HPP_
#define BLOWMORPH_CLIENT_NETWORK_CONTROLLER_HPP_

#include <cstdlib>

#include <list>
#include <string>

namespace bm {

class NetworkController {
public:
  class Listener {
  public:
    Listener();
    ~Listener();

    virtual bool OnConnect() = 0;
    virtual bool OnDisconnect() = 0;

    virtual bool OnMessageSent() = 0;
    virtual bool OnMessageReceived() = 0;
  };

public:
  NetworkController();
  ~NetworkController();

  bool Initialize(const std::string& ip, int port);
  bool Finalize();

  bool Service(/*TODO*/);

  bool Connect();
  bool Disconnect();

  bool SendMessage(const char* message, size_t length);

  void RegisterListener(Listener* listener);
  void UnregisterListener(Listener* listener);

private:
  std::string _ip;
  int _port;

  std::list<Listener*> _listeners;

  enum {
    STATE_DISCONNECTED,
    STATE_CONNECTED
  } _state;
};

} // namespace bm

#endif // BLOWMORPH_CLIENT_NETWORK_CONTROLLER_HPP_
