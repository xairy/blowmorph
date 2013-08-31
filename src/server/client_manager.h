#ifndef BLOWMORPH_SERVER_CLIENT_MANAGER_H_
#define BLOWMORPH_SERVER_CLIENT_MANAGER_H_

#include <map>
#include <vector>

#include <enet-plus/enet.hpp>

#include <base/pstdint.h>

#include "entity.h"

namespace bm {

struct Client {
  Client(enet::Peer* peer, Player* entity);
  ~Client();

  enet::Peer* peer;
  Player* entity;
};

class ClientManager {
public:
  ClientManager();
  ~ClientManager();

  void AddClient(uint32_t id, Client* client);

  Client* GetClient(uint32_t id);
  void DeleteClient(uint32_t id, bool deallocate);
  void DisconnectClient(uint32_t id);

  std::map<uint32_t, Client*>* GetClients();
  void DeleteClients(const std::vector<uint32_t>& input, bool deallocate);
  void DisconnectClients(const std::vector<uint32_t>& input);

private:
  std::map<uint32_t, Client*> _clients;
};

} // namespace bm

#endif // BLOWMORPH_SERVER_CLIENT_MANAGER_H_
