#include "network_controller.hpp"

#include <cstdlib>

#include <algorithm>
#include <list>
#include <memory>
#include <vector>
#include <string>

#include <enet-wrapper/enet.hpp>

#include <base/error.hpp>
#include <base/macros.hpp>
#include <base/pstdint.hpp>
#include <base/timer.hpp>

namespace bm {

NetworkController::NetworkController() : _state(STATE_FINALIZED) { }
NetworkController::~NetworkController() {
  CHECK(_state == STATE_FINALIZED);
  // XXX: network state?
}

bool NetworkController::Initialize(const std::string& host, int port) {
  CHECK(_state == STATE_FINALIZED);
  CHECK(port > 0);

  _host = host;
  _port = port;

  if(!_enet.Initialize()) {
    BM_ERROR("Could not initialize ENet!");
    return false;
  }

  std::auto_ptr<ClientHost> client(_enet.CreateClientHost());
  if(client.get() == NULL) {
    BM_ERROR("Could not create client host!");
    return false;
  }

  std::auto_ptr<Event> event(_enet.CreateEvent());
  if(event.get() == NULL) {
    BM_ERROR("Could not create event!");
    return false;
  }

  _client = client.release();
  _event = event.release();
  _peer = NULL;

  _network_state = NETWORK_STATE_DISCONNECTED;
  _state = STATE_INITIALIZED;
  return true;
}

void NetworkController::Finalize() {
  CHECK(_state == STATE_INITIALIZED);

  if(_client != NULL) {
    delete _client;
    _client = NULL;
  }
  if(_peer != NULL) {
    delete _peer;
    _peer = NULL;
  }
  if(_event != NULL) {
    delete _event;
    _event = NULL;
  }

  _state = STATE_FINALIZED;
}

// XXX[xairy 26.09.2012]: services all incoming messages.
bool NetworkController::Service(/*uint32_t timeout*/) {
  CHECK(_state == STATE_INITIALIZED);
  CHECK(_network_state == NETWORK_STATE_CONNECTED);

  std::vector<char> message;
  
  do {
    bool rv = _client->Service(_event, /*timeout*/0);
    if(rv == false) {
      BM_ERROR("Could not service client host!");
      return false;
    }
    switch(_event->GetType()) {
      case Event::EVENT_RECEIVE: {
        _event->GetData(&message);
        _event->DestroyPacket();

        std::list<Listener*>::iterator itr;
        for(itr = _listeners.begin(); itr != _listeners.end(); ++itr) {
          (*itr)->OnMessageReceived(&message[0], message.size());
        }
      } break;
      
      case Event::EVENT_CONNECT: {
        BM_ERROR("Got EVENT_CONNECT while being already connected.");
        return false;
      } break;

      case Event::EVENT_DISCONNECT: {
        _network_state = NETWORK_STATE_DISCONNECTED;

        std::list<Listener*>::iterator itr;
        for(itr = _listeners.begin(); itr != _listeners.end(); ++itr) {
          (*itr)->OnDisconnect();
        }

        // XXX[xairy, 26.09.2012]: Error, false?
        BM_ERROR("Disconnected from server!");
        return false;
      } break;
    }
  } while (_event->GetType() != Event::EVENT_NONE);
  
  return true;
}

bool NetworkController::Connect(uint32_t timeout) {
  CHECK(_state == STATE_INITIALIZED);
  CHECK(_network_state == NETWORK_STATE_DISCONNECTED);

  std::auto_ptr<Peer> peer(_client->Connect(_host, _port));
  if(peer.get() == NULL) {
    return false;
  }

  bool rv = _client->Service(_event, timeout);
  if(rv == false) {
    BM_ERROR("Could not service client host!");
    return false;
  }
  if(_event->GetType() != Event::EVENT_CONNECT) {
    BM_ERROR("Got something else while waiting for EVENT_CONNECT!");
    return false;
  }

  _peer = peer.release();

  _network_state = NETWORK_STATE_CONNECTED;

  std::list<Listener*>::iterator itr;
  for(itr = _listeners.begin(); itr != _listeners.end(); ++itr) {
    (*itr)->OnConnect();
  }

  return true;
}

bool NetworkController::Disconnect(uint32_t timeout) {
  _peer->Disconnect();

  Timer timer;
  bm::uint32_t start = timer.GetTime();
  while(timer.GetTime() - start <= timeout) {
    bool rv = _client->Service(_event, timeout);
    if(rv == false) {
      BM_ERROR("Could not service client host!");
      return false;
    }
    if(_event != NULL && _event->GetType() == Event::EVENT_DISCONNECT) {
      return true;
    }
  }

  BM_ERROR("Not received EVENT_DISCONNECT event while disconnecting.");
  return false;
}

bool NetworkController::SendMessage(const char* message, size_t length) {
  // XXX[xairy 26.09.2012]: always sending reliable messages through channel 0.
  bool rv = _peer->Send(message, length, true, 0);
  if(rv == false) {
    BM_ERROR("Could not send message to server!");
    return false;
  }
  std::list<Listener*>::iterator itr;
  for(itr = _listeners.begin(); itr != _listeners.end(); ++itr) {
    // XXX[xairy 26.09.2012]: message was not sent, actually.
    (*itr)->OnMessageSent(message, length);
  }
  return true;
}

bool NetworkController::RegisterListener(Listener* listener) {
  std::list<Listener*>::iterator itr =
    std::find(_listeners.begin(), _listeners.end(), listener);
  if(itr != _listeners.end()) {
    BM_ERROR("Nework controller listener has already been registered!");
    return false;
  }
  _listeners.push_back(listener);
  return true;
}

bool NetworkController::UnregisterListener(Listener* listener) {
  std::list<Listener*>::iterator itr =
    std::find(_listeners.begin(), _listeners.end(), listener);
  if(itr != _listeners.end()) {
    _listeners.erase(itr);
    return true;
  }
  BM_ERROR("Network controller listener has not been registered!");
  return false;
}

}
