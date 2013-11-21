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
  Object(const sf::Vector2f& position, int64_t time, uint32_t id,
      uint32_t type, const std::string& path, int64_t time_offset);

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
  Sprite sprite;
  bool visible;
  bool name_visible;
  sf::Vector2f name_offset;
  bool interpolation_enabled;
  ObjectInterpolator interpolator;
};

// XXX(xairy): separate function?
void RenderObject(Object* object, int64_t time,
    sf::Font* font, sf::RenderWindow& render_window);

}  // namespace bm

#endif  // CLIENT_OBJECT_H_
