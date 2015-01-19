// Copyright (c) 2013 Blowmorph Team

#include "client/object.h"

#include <string>

#include <SFML/Graphics.hpp>

#include "interpolator/interpolator.h"

#include "base/body.h"
#include "base/macros.h"
#include "base/pstdint.h"
#include "base/time.h"

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
Object::Object(uint32_t id, Type type, b2World* world, Sprite* sprite,
    const sf::Vector2f& position, int64_t time)
      : id(id),
        type(type),
        visible(true),
        sprite(sprite),
        caption_visible(false) {
  // TODO(xairy): initialize sprite here.

  SettingsManager body_settings;
  bool rv = body_settings.Open("data/bodies.cfg");
  CHECK(rv == true);

  std::string body_config;
  switch (type) {
    case TYPE_BULLET:
      body_config = "bullet";
      break;
    case TYPE_PLAYER:
      body_config = "mechos";
      break;
    case TYPE_WALL:
      body_config = "wall";
      break;
    case TYPE_KIT:
      body_config = "kit";
      break;
    case TYPE_DUMMY:
      body_config = "dummy";
      break;
    default:
      CHECK(false);  // Unreachable.
  }

  body.Create(world, &body_settings, body_config);
  body.SetUserData(this);
  body.SetPosition(b2Vec2(position.x, position.y));
}

Object::~Object() {
  if (sprite != NULL) delete sprite;
}

Object::Type Object::GetType() const {
  return type;
}

void Object::EnableCaption(const std::string& caption, const sf::Font& font) {
  CHECK(caption_visible == false);
  caption_text = sf::Text(caption, font, 12);
  sf::FloatRect rect = caption_text.getLocalBounds();
  sf::Vector2f origin(rect.left + rect.width / 2, rect.top  + rect.height / 2);
  caption_text.setOrigin(Round(origin));
  caption_visible = true;
}

sf::Vector2f Object::GetPosition() {
  b2Vec2 b2p = body.GetPosition();
  return sf::Vector2f(b2p.x, b2p.y);
}

void Object::SetPosition(const sf::Vector2f& position) {
  body.SetPosition(b2Vec2(position.x, position.y));
}

// TODO(xairy): use ApplyImpulse instead of SetVelocity everywhere.
void Object::SetInterpolationPosition(const sf::Vector2f& position,
    int64_t snapshot_time, int64_t interpolation_offset, int64_t server_time) {
  // Impulse to compensate the current velocity.
  b2Vec2 velocity = body.GetVelocity();
  float mass = body.GetMass();
  b2Vec2 velocity_impulse = -mass * velocity;

  CHECK(server_time - interpolation_offset < snapshot_time);
  b2Vec2 interpolation_impulse = mass /
      (snapshot_time - (server_time - interpolation_offset)) * 1000 *
      (b2Vec2(position.x, position.y) - body.GetPosition());

  body.ApplyImpulse(velocity_impulse + interpolation_impulse);
}

void Object::Render(sf::RenderWindow& render_window, int64_t time) {
  b2Vec2 b2p = body.GetPosition();
  sf::Vector2f position = Round(sf::Vector2f(b2p.x, b2p.y));

  if (visible) {
    sprite->SetPosition(position);
    sprite->Render(&render_window);
  }

  if (visible && caption_visible) {
    sf::Vector2f caption_offset = sf::Vector2f(0.0f, -25.0f);
    sf::Vector2f caption_pos = position + caption_offset;
    caption_text.setPosition(caption_pos.x, caption_pos.y);
    render_window.draw(caption_text);
  }
}

}  // namespace bm
