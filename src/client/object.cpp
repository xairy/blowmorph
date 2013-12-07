// Copyright (c) 2013 Blowmorph Team

#include "client/object.h"

#include <string>

#include <SFML/Graphics.hpp>

#include "interpolator/interpolator.h"

#include "base/macros.h"
#include "base/pstdint.h"

namespace {

float Length(const sf::Vector2f& vector) {
  return sqrt(vector.x * vector.x + vector.y * vector.y);
}

sf::Vector2f Round(const sf::Vector2f& vector) {
  return sf::Vector2f(floor(vector.x), floor(vector.y));
}

}  // anonymous namespace

namespace interpolator {

template<>
sf::Vector2f lerp(const sf::Vector2f& a, const sf::Vector2f& b, double ratio) {
  sf::Vector2f result;
  result.x = lerp(a.x, b.x, ratio);
  result.y = lerp(a.y, b.y, ratio);
  return result;
}

template<>
bm::ObjectState lerp(const bm::ObjectState& a,
    const bm::ObjectState& b, double ratio) {
  bm::ObjectState result;
  result.position = lerp(a.position, b.position, ratio);
  result.blow_charge = lerp(a.blow_charge, b.blow_charge, ratio);
  result.morph_charge = lerp(a.morph_charge, b.morph_charge, ratio);
  result.health = lerp(a.health, b.health, ratio);
  return result;
}

}  // namespace interpolator

namespace bm {

// TODO(alex): fix method names.
Object::Object(uint32_t id, uint32_t type, Sprite* sprite,
    const sf::Vector2f& position, int64_t time)
      : id(id),
        type(type),
        visible(true),
        sprite(sprite),
        caption_visible(false),
        interpolation_enabled(false),
        interpolator(ObjectInterpolator(0, 1)) {
  // XXX(xairy): call some method instead?
  ObjectState state;
  state.position = position;
  state.blow_charge = 0;
  state.morph_charge = 0;
  state.health = 0;
  interpolator.Push(state, time);
}

void Object::ShowCaption(const std::string& caption, const sf::Font& font) {
  CHECK(caption_visible == false);
  caption_text = sf::Text(caption, font, 12);
  sf::FloatRect rect = caption_text.getLocalBounds();
  sf::Vector2f origin(rect.left + rect.width / 2, rect.top  + rect.height / 2);
  caption_text.setOrigin(Round(origin));
  caption_visible = true;
}

void Object::EnableInterpolation(int64_t interpolation_offset) {
  CHECK(interpolation_enabled == false);
  interpolator = ObjectInterpolator(interpolation_offset, 2);
  interpolation_enabled = true;
}

void Object::EnforceState(const ObjectState& state, int64_t time) {
  interpolator.Clear();
  interpolator.Push(state, time);
}

void Object::PushState(const ObjectState& state, int64_t time) {
  interpolator.Push(state, time);
}

sf::Vector2f Object::GetPosition(int64_t time) {
  return interpolator.Interpolate(time).position;
}

void Object::SetPosition(const sf::Vector2f& value, int64_t time) {
  ObjectState state = interpolator.Interpolate(time);
  state.position = value;
  EnforceState(state, time);
}

void Object::Move(const sf::Vector2f& value, int64_t time) {
  ObjectState state = interpolator.Interpolate(time);
  state.position = state.position + value;
  EnforceState(state, time);
}

void RenderObject(Object* object, int64_t time,
    sf::Font* font, sf::RenderWindow& render_window) {
  CHECK(object != NULL);
  CHECK(font != NULL);

  ObjectState state = object->interpolator.Interpolate(time);

  if (object->visible) {
    sf::Vector2f object_pos(Round(state.position));
    object->sprite->SetPosition(object_pos);
    object->sprite->Render(&render_window);
  }

  if (object->visible && object->caption_visible) {
    sf::Vector2f caption_offset = sf::Vector2f(0.0f, -25.0f);
    sf::Vector2f caption_pos = Round(state.position + caption_offset);
    object->caption_text.setPosition(caption_pos.x, caption_pos.y);
    render_window.draw(object->caption_text);
  }
}

}  // namespace bm
