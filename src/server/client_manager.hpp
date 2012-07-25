#ifndef BLOWMORPH_SERVER_CLIENT_MANAGER_HPP_
#define BLOWMORPH_SERVER_CLIENT_MANAGER_HPP_

#include <map>

#include "base/macros.hpp"
#include "base/pstdint.hpp"

#include "entity.hpp"

namespace bm {

struct Client {
  Client(Peer* peer, Player* entity) : peer(peer), entity(entity) {
    CHECK(peer != NULL);
  }
  ~Client() {
    delete peer;
  }

  Peer* peer;
  Player* entity;
};

class ClientManager {
public:
  ClientManager() { }
  ~ClientManager() {
    std::map<uint32_t, Client*>::iterator i;
    for(i = _clients.begin(); i != _clients.end(); ++i) {
      delete i->second;
    }
  }

  void AddClient(uint32_t id, Client* client) {
    CHECK(_clients.count(id) == 0);
    _clients[id] = client;
  }

  Client* GetClient(uint32_t id) {
    CHECK(_clients.count(id) == 1);
    return _clients[id];
  }

  void DeleteClient(uint32_t id, bool deallocate) {
    CHECK(_clients.count(id) == 1);
    if(deallocate) {
      delete _clients[id];
    }
    _clients.erase(id);
  }

  void DisconnectClient(uint32_t id) {
    CHECK(_clients.count(id) == 1);
    _clients[id]->peer->Disconnect();
  }

  std::map<uint32_t, Client*>* GetClients() {
    return &_clients;
  }

  void DeleteClients(const std::vector<uint32_t>& input, bool deallocate) {
    size_t size = input.size();
    for(size_t i = 0; i < size; i++) {
      DeleteClient(input[i], deallocate);
    }
  }

  void DisconnectClients(const std::vector<uint32_t>& input) {
    size_t size = input.size();
    for(size_t i = 0; i < size; i++) {
      DisconnectClient(input[i]);
    }
  }

private:
  std::map<uint32_t, Client*> _clients;
};

} // namespace bm

#endif // BLOWMORPH_SERVER_CLIENT_MANAGER_HPP_
