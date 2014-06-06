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
  float blow_charge;
  float morph_charge;
  float health;
};

typedef interpolator::LinearInterpolator<ObjectState, int64_t>
  ObjectInterpolator;

// TODO(alex): fix method names.
// TODO(xairy): comments.
struct Object {
  Object(uint32_t id, Sprite* sprite,
    const sf::Vector2f& position, int64_t time);
  ~Object();

  void ShowCaption(const std::string& caption, const sf::Font& font);

  // Position will be reset after enabling interpolation.
  void EnableInterpolation(int64_t interpolation_offset);

  void EnforceState(const ObjectState& state, int64_t time);
  void PushState(const ObjectState& state, int64_t time);

  sf::Vector2f GetPosition(int64_t time = 0);
  void SetPosition(const sf::Vector2f& value, int64_t time = 0);
  void Move(const sf::Vector2f& value, int64_t time = 0);

  uint32_t id;

  bool visible;
  Sprite* sprite;
  //Body* body;

  bool caption_visible;
  sf::Text caption_text;

  bool interpolation_enabled;
  ObjectInterpolator interpolator;
};

// XXX(xairy): separate function?
void RenderObject(Object* object, int64_t time,
    sf::Font* font, sf::RenderWindow& render_window);

}  // namespace bm

#endif  // CLIENT_OBJECT_H_

