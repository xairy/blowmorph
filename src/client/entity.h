// Copyright (c) 2013 Blowmorph Team

#ifndef CLIENT_ENTITY_H_
#define CLIENT_ENTITY_H_

#include <cmath>

#include <string>

#include <SFML/Graphics.hpp>

#include "base/body.h"
#include "base/pstdint.h"

#include "client/sprite.h"

namespace bm {

// TODO(xairy): merge with server's Entity class.
class Entity {
 public:
  enum Type {
    TYPE_PLAYER,
    TYPE_BULLET,
    TYPE_WALL,
    TYPE_KIT,
    TYPE_DUMMY,
    TYPE_ACTIVATOR
  };

  Entity(
    SettingsManager *body_settings,
    SettingsManager *entity_settings,
    uint32_t id,
    Type type,
    b2World *world,
    Sprite* sprite,
    const b2Vec2& position,
    int64_t time);
  ~Entity();

  Type GetType() const;
  uint32_t GetId() const;

  Body* GetBody();

  // Makes object to start moving towards 'position' to be there when
  // current time - 'interpolation_offset' == 'time'.
  void SetInterpolationPosition(const b2Vec2& position,
      int64_t snapshot_time, int64_t interpolation_offset, int64_t server_time);

  void EnableCaption(const std::string& caption, const sf::Font& font);

  void Render(sf::RenderWindow* render_window, int64_t time);

 private:
  uint32_t id_;
  Type type_;

  bool visible_;
  Sprite* sprite_;
  Body body_;

  bool caption_visible_;
  sf::Text caption_text_;
};

}  // namespace bm

#endif  // CLIENT_ENTITY_H_
