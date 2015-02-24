// Copyright (c) 2015 Blowmorph Team

#ifndef CLIENT_RENDER_WINDOW_H_
#define CLIENT_RENDER_WINDOW_H_

#include <map>
#include <string>

#include <SFML/Graphics.hpp>

#include "base/macros.h"
#include "base/pstdint.h"

#include "engine/config.h"
#include "engine/world.h"

#include "client/entity.h"
#include "client/resource_manager.h"
#include "client/sprite.h"

namespace bm {

class RenderWindow {
 public:
  RenderWindow();
  ~RenderWindow();

  void Initialize();

  void StartFrame();
  void EndFrame();

  void SetViewCenter(sf::Vector2f location);

  void RenderTerrain(Sprite* sprite);

  void RenderSprite(Sprite* sprite);
  void RenderEntity(ClientEntity* entity);
  void RenderWorld(World* world);

  void RenderPlayerStats(
    int health, int max_health,
    int energy, int max_energy);
  void RenderMinimap(World* world, ClientEntity* player);
  void RenderScoretable(
    std::map<uint32_t, int> player_scores,
    std::map<uint32_t, std::string> player_names);

  void RenderText(
    const std::string& str,
    const sf::Vector2f& position,
    int size, sf::Color color);

  sf::Vector2u GetWindowSize() const;
  sf::Font* GetFont();
  sf::Vector2i GetMousePosition() const;

  bool PollEvent(sf::Event* event);

 private:
  sf::RenderWindow* render_window_;
  sf::View view_;
  sf::Font* font_;

  enum {
    STATE_FINALIZED,
    STATE_INITIALIZED
  } state_;

  DISALLOW_COPY_AND_ASSIGN(RenderWindow);
};

}  // namespace bm

#endif  // CLIENT_RENDER_WINDOW_H_
