// Copyright (c) 2013 Blowmorph Team

#include "client/entity.h"

#include <string>

#include <SFML/Graphics.hpp>

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

namespace bm {

Entity::Entity(
  SettingsManager *body_settings,
  SettingsManager *entity_settings,
  uint32_t id,
  Type type,
  b2World* world,
  Sprite* sprite,
  const b2Vec2& position,
  int64_t time
) : id_(id),
    type_(type),
    visible_(true),
    sprite_(sprite),
    caption_visible_(false) {
  // TODO(xairy): initialize sprite here.

  std::string body_config;
  switch (type) {
    case TYPE_BULLET:
      body_config = "bullet";
      break;
    case TYPE_PLAYER:
      body_config = "man";
      break;
    case TYPE_WALL:
      body_config = "wall";
      break;
    case TYPE_KIT:
      body_config = "kit";
      break;
    case TYPE_DUMMY:
      body_config = "zombie";
      break;
    default:
      CHECK(false);  // Unreachable.
  }

  body_.Create(world, body_settings, body_config);
  body_.SetUserData(this);
  body_.SetPosition(position);
}

Entity::~Entity() {
  if (sprite_ != NULL) delete sprite_;
}

Entity::Type Entity::GetType() const {
  return type_;
}

uint32_t Entity::GetId() const {
  return id_;
}

Body* Entity::GetBody() {
  return &body_;
}

void Entity::SetInterpolationPosition(const b2Vec2& position,
    int64_t snapshot_time, int64_t interpolation_offset, int64_t server_time) {
  CHECK(server_time - interpolation_offset < snapshot_time);
  b2Vec2 impulse = body_.GetMass() /
      (snapshot_time - (server_time - interpolation_offset)) * 1000 *
      (b2Vec2(position.x, position.y) - body_.GetPosition());
  body_.SetImpulse(impulse);
}

void Entity::EnableCaption(const std::string& caption, const sf::Font& font) {
  CHECK(caption_visible_ == false);
  caption_text_ = sf::Text(caption, font, 12);
  sf::FloatRect rect = caption_text_.getLocalBounds();
  sf::Vector2f origin(rect.left + rect.width / 2, rect.top  + rect.height / 2);
  caption_text_.setOrigin(Round(origin));
  caption_visible_ = true;
}

void Entity::Render(sf::RenderWindow* render_window, int64_t time) {
  b2Vec2 b2p = body_.GetPosition();
  sf::Vector2f position = Round(sf::Vector2f(b2p.x, b2p.y));

  if (visible_) {
    sprite_->SetPosition(position);
    sprite_->SetRotation(body_.GetRotation());
    sprite_->Render(render_window);
  }

  if (visible_ && caption_visible_) {
    sf::Vector2f caption_offset = sf::Vector2f(0.0f, -25.0f);
    sf::Vector2f caption_pos = position + caption_offset;
    caption_text_.setPosition(caption_pos.x, caption_pos.y);
    render_window->draw(caption_text_);
  }
}

}  // namespace bm
