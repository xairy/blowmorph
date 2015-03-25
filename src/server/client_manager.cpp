// Copyright (c) 2013 Blowmorph Team

#include "server/client_manager.h"

#include <map>
#include <string>
#include <vector>

#include "base/macros.h"
#include "base/pstdint.h"

#include "net/enet.h"

#include "server/entity.h"

namespace bm {

Client::Client(Peer* peer, Player* entity, const std::string& login)
    : peer(peer), entity(entity), login(login) {
  CHECK(peer != NULL);
}
Client::~Client() { }

ClientManager::ClientManager() { }
ClientManager::~ClientManager() {
  std::map<uint32_t, Client*>::iterator i;
  for (i = _clients.begin(); i != _clients.end(); ++i) {
    delete i->second;
  }
}

void ClientManager::AddClient(uint32_t id, Client* client) {
  CHECK(_clients.count(id) == 0);
  _clients[id] = client;
}

Client* ClientManager::GetClient(uint32_t id) {
  CHECK(_clients.count(id) == 1);
  return _clients[id];
}

void ClientManager::DeleteClient(uint32_t id, bool deallocate) {
  CHECK(_clients.count(id) == 1);
  if (deallocate) {
    delete _clients[id];
  }
  _clients.erase(id);
}

void ClientManager::DisconnectClient(uint32_t id) {
  CHECK(_clients.count(id) == 1);
  _clients[id]->peer->Disconnect();
}

std::map<uint32_t, Client*>* ClientManager::GetClients() {
  return &_clients;
}

void ClientManager::DeleteClients(const std::vector<uint32_t>& input,
    bool deallocate) {
  size_t size = input.size();
  for (size_t i = 0; i < size; i++) {
    DeleteClient(input[i], deallocate);
  }
}

void ClientManager::DisconnectClients(const std::vector<uint32_t>& input) {
  size_t size = input.size();
  for (size_t i = 0; i < size; i++) {
    DisconnectClient(input[i]);
  }
}

}  // namespace bm
