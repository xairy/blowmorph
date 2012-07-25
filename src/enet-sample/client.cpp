#include "enet-wrapper/enet.hpp"

#include "base/error.hpp"

using namespace bm;

int main() {
  Enet enet;

  ClientHost* client = enet.CreateClientHost();
  CHECK(client != NULL);

  Peer* peer = client->Connect("5.41.35.100", 4242);
  CHECK(peer != NULL);
 
  Event* event = enet.CreateEvent();

  bm::uint32_t timeout = 5000;

  bool rv = client->Service(event, timeout);
  CHECK(rv);
  CHECK(event->GetType() == Event::EVENT_CONNECT);

  printf("Connected to %s:%u.\n", event->GetPeerIp().c_str(), event->GetPeerPort());

  rv = client->Service(event, timeout);
  CHECK(rv);
  CHECK(event->GetType() == Event::EVENT_RECEIVE);

  std::vector<char> msg;
  event->GetData(&msg);
  msg.push_back(0);

  printf("Message received: %s\n", &msg[0]);
  
  char message[] = "Hello world!";
  rv = peer->Send(message, sizeof(message));
  CHECK(rv);

  client->Flush();

  printf("Message sent: %s\n", message);

  peer->Disconnect();

  rv = client->Service(event, timeout);
  CHECK(rv);
  CHECK(event->GetType() == Event::EVENT_DISCONNECT);

  printf("Disconnected from %s:%u.\n", event->GetPeerIp().c_str(), event->GetPeerPort());

  delete event;
  delete peer;

  client->Destroy();
  delete client;

  while(true);

  return 0;
}
