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
  return sf::Vector2f(round(vector.x), round(vector.y));
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
  result.blowCharge = lerp(a.blowCharge, b.blowCharge, ratio);
  result.morphCharge = lerp(a.morphCharge, b.morphCharge, ratio);
  result.health = lerp(a.health, b.health, ratio);
  return result;
}

}  // namespace interpolator

namespace bm {

// TODO(alex): fix method names.
Object::Object(uint32_t id, uint32_t type, Sprite* sprite,
    const sf::Vector2f& position, int64_t time, int64_t time_offset,
    const std::string& name, sf::Font* font)
     : id(id),
        type(type),
        visible(false),
        sprite(sprite),
        name_visible(false),
        name_text(sf::Text(name, *font, 12)),
        interpolation_enabled(false),
        interpolator(ObjectInterpolator(time_offset, 1)) {
  sf::FloatRect rect = name_text.getLocalBounds();
  sf::Vector2f origin(rect.left + rect.width / 2, rect.top  + rect.height / 2);
  name_text.setOrigin(Round(origin));

  ObjectState state;
  state.blowCharge = 0;
  state.health = 0;
  state.morphCharge = 0;
  state.position = position;
  interpolator.Push(state, time);
}

void Object::EnableInterpolation() {
  interpolation_enabled = true;
  interpolator.SetFrameCount(2);
}

void Object::DisableInterpolation() {
  interpolation_enabled = false;
  interpolator.SetFrameCount(1);
}

void Object::EnforceState(const ObjectState& state, int64_t time) {
  interpolator.Clear();
  interpolator.Push(state, time);
}

void Object::UpdateState(const ObjectState& state, int64_t time) {
  interpolator.Push(state, time);
}

sf::Vector2f Object::GetPosition(int64_t time) {
  return interpolator.Interpolate(time).position;
}

sf::Vector2f Object::GetPosition() {
  CHECK(!interpolation_enabled);
  return GetPosition(0);
}

void Object::SetPosition(const sf::Vector2f& value) {
  CHECK(!interpolation_enabled);
  ObjectState state = interpolator.Interpolate(0);
  state.position = value;
  EnforceState(state, 0);
}

void Object::Move(const sf::Vector2f& value) {
  CHECK(!interpolation_enabled);
  ObjectState state = interpolator.Interpolate(0);
  state.position = state.position + value;
  EnforceState(state, 0);
}

void RenderObject(Object* object, int64_t time,
    sf::Font* font, sf::RenderWindow& render_window) {
  CHECK(object != NULL);
  CHECK(font != NULL);

  ObjectState state = object->interpolator.Interpolate(time);

  if (object->visible) {
    sf::Vector2f object_pos(round(state.position.x), round(state.position.y));
    object->sprite->SetPosition(object_pos);
    object->sprite->Render(&render_window);
  }

  if (object->visible && object->name_visible) {
    sf::Vector2f name_offset = sf::Vector2f(0.0f, -25.0f);
    sf::Vector2f name_pos = Round(state.position + name_offset);
    object->name_text.setPosition(name_pos.x, name_pos.y);
    render_window.draw(object->name_text);
  }
}

}  // namespace bm
