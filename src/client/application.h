// Copyright (c) 2013 Blowmorph Team

#ifndef CLIENT_APPLICATION_H_
#define CLIENT_APPLICATION_H_

#include <list>
#include <map>
#include <string>
#include <vector>

#include <Box2D/Box2D.h>
#include <SFML/Graphics.hpp>

#include <enet-plus/enet.h>

#include "base/macros.h"
#include "base/protocol.h"
#include "base/pstdint.h"
#include "base/settings_manager.h"

#include "client/contact_listener.h"
#include "client/entity.h"
#include "client/resource_manager.h"
#include "client/sprite.h"

namespace bm {

class Application {
 public:
  Application();
  ~Application();

  bool Initialize();
  bool Run();
  void Finalize();

 private:
  bool InitializeGraphics();
  bool InitializePhysics();
  bool InitializeNetwork();

  bool Connect();
  bool Synchronize();

  // Returns approximate server time.
  int64_t GetServerTime();

  bool PumpEvents();
  bool ProcessEvent(const sf::Event& event);

  bool OnQuitEvent();
  bool OnMouseButtonEvent(const sf::Event& event);
  bool OnKeyEvent(const sf::Event& event);

  bool PumpPackets();
  bool ProcessPacket(const std::vector<char>& buffer);

  void OnEntityAppearance(const EntitySnapshot* snapshot);
  void OnEntityUpdate(const EntitySnapshot* snapshot);
  void OnPlayerUpdate(const EntitySnapshot* snapshot);
  bool OnEntityDisappearance(const EntitySnapshot* snapshot);

  void SimulatePhysics();

  void Render();
  void RenderHUD();
  void WriteText(const std::string& str, const sf::Vector2f& position,
      int size, sf::Color color);

  bool SendInputEvents();

  bool is_running_;

  SettingsManager client_settings_;
  SettingsManager entity_settings_;
  SettingsManager body_settings_;
  ResourceManager resource_manager_;

  sf::RenderWindow* render_window_;
  sf::View view_;
  sf::Font* font_;

  enet::Enet enet_;
  enet::ClientHost* client_;
  enet::Event* event_;
  enet::Peer* peer_;

  int tick_rate_;

  int64_t latency_;
  int64_t time_correction_;

  int64_t last_tick_;
  int64_t last_physics_simulation_;

  Entity* player_;
  std::map<uint32_t, Entity*> dynamic_entities_;
  std::map<uint32_t, Entity*> static_entities_;
  std::list<Sprite*> explosions_;

  b2World* world_;
  ContactListener contact_listener_;

  ClientOptions client_options_;

  std::map<uint32_t, std::string> player_names_;

  bool show_score_table_;
  std::map<uint32_t, int> player_scores_;

  float player_health_;
  float player_blow_charge_;
  float player_morph_charge_;

  float max_player_misposition_;
  int64_t interpolation_offset_;

  // Input events since the last tick.
  std::vector<KeyboardEvent> keyboard_events_;
  std::vector<MouseEvent> mouse_events_;

  struct KeyboardState {
    KeyboardState() : up(false), down(false), right(false), left(false) { }

    bool up;
    bool down;
    bool right;
    bool left;
  };
  KeyboardState keyboard_state_;

  enum NetworkState {
    NETWORK_STATE_DISCONNECTED,
    NETWORK_STATE_INITIALIZED,
    NETWORK_STATE_CONNECTED,
    // TODO(xairy): rename to _SYNCHRONIZED.
    NETWORK_STATE_LOGGED_IN
  };
  NetworkState network_state_;

  enum State {
    STATE_INITIALIZED,
    STATE_FINALIZED
  };
  State state_;

  DISALLOW_COPY_AND_ASSIGN(Application);
};

}  // namespace bm

#endif  // CLIENT_APPLICATION_H_
