// Copyright (c) 2013 Blowmorph Team

#ifndef CLIENT_OBJECT_H_
#define CLIENT_OBJECT_H_

#include <cmath>

#include <string>

#include <SFML/Graphics.hpp>

#include "interpolator/interpolator.h"

#include "base/pstdint.h"

#include "client/sprite.h"

namespace bm {

struct ObjectState {
  sf::Vector2f position;
  float blowCharge;
  float morphCharge;
  float health;
};

typedef interpolator::LinearInterpolator<ObjectState, int64_t>
  ObjectInterpolator;

// TODO(alex): fix method names.
struct Object {
  Object(uint32_t id, uint32_t type, Sprite* sprite,
    const sf::Vector2f& position, int64_t time, int64_t time_offset,
    const std::string& name = "", sf::Font* font = NULL);

  void EnableInterpolation();
  void DisableInterpolation();

  void EnforceState(const ObjectState& state, int64_t time);
  void UpdateState(const ObjectState& state, int64_t time);

  // TODO(xairy): comment the difference.
  sf::Vector2f GetPosition(int64_t time);
  sf::Vector2f GetPosition();

  void SetPosition(const sf::Vector2f& value);
  void Move(const sf::Vector2f& value);

  uint32_t id;
  uint32_t type;

  bool visible;
  Sprite* sprite;

  bool name_visible;
  sf::Text name_text;

  bool interpolation_enabled;
  ObjectInterpolator interpolator;
};

// XXX(xairy): separate function?
void RenderObject(Object* object, int64_t time,
    sf::Font* font, sf::RenderWindow& render_window);

}  // namespace bm

#endif  // CLIENT_OBJECT_H_
