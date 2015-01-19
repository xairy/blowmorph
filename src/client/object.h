// Copyright (c) 2013 Blowmorph Team

#ifndef CLIENT_OBJECT_H_
#define CLIENT_OBJECT_H_

#include <cmath>

#include <string>

#include <SFML/Graphics.hpp>

#include "interpolator/interpolator.h"

#include "base/body.h"
#include "base/pstdint.h"

#include "client/sprite.h"

namespace bm {

struct ObjectState {
  sf::Vector2f position;
  float blow_charge;
  float morph_charge;
  float health;
};

typedef interpolator::LinearInterpolator<ObjectState, int64_t>
  ObjectInterpolator;

// TODO(xairy): merge with server's Entity class.
struct Object {
  enum Type {
    TYPE_PLAYER,
    TYPE_BULLET,
    TYPE_WALL,
    TYPE_KIT,
    TYPE_DUMMY
  };

  Object(uint32_t id, Type type, b2World *world, Sprite* sprite,
    const sf::Vector2f& position, int64_t time);
  ~Object();

  Type GetType() const;

  void EnableCaption(const std::string& caption, const sf::Font& font);

  sf::Vector2f GetPosition();
  void SetPosition(const sf::Vector2f& position);

  // Makes object move towards 'position' to be there when
  // current time - 'interpolation_offset' == 'time'.
  void SetInterpolationPosition(const sf::Vector2f& position,
      int64_t snapshot_time, int64_t interpolation_offset, int64_t server_time);

  void Render(sf::RenderWindow& render_window, int64_t time);

  uint32_t id;
  Type type;

  bool visible;
  Sprite* sprite;
  Body body;

  bool caption_visible;
  sf::Text caption_text;
};

}  // namespace bm

#endif  // CLIENT_OBJECT_H_

