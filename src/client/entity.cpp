// Copyright (c) 2015 Blowmorph Team

#include "client/entity.h"

#include <cmath>

#include <string>

#include <SFML/Graphics.hpp>

#include "base/macros.h"
#include "base/pstdint.h"
#include "base/time.h"
#include "base/utils.h"

#include "engine/body.h"
#include "engine/entity.h"

#include "client/sprite.h"

namespace {

sf::Vector2f Round(const sf::Vector2f& vector) {
  return sf::Vector2f(floor(vector.x), floor(vector.y));
}

}  // anonymous namespace

namespace bm {

ClientEntity::ClientEntity(
  b2World* world,
  uint32_t id,
  Type type,
  const std::string& entity_name,
  b2Vec2 position,
  Sprite* sprite
) : Entity(world, id, type, entity_name, position, FILTER_DEFAULT, FILTER_ALL),
    visible_(true),
    sprite_(sprite),
    caption_visible_(false) {
  // XXX(xairy): create Sprite here?
}

ClientEntity::~ClientEntity() {
  if (sprite_ != NULL) delete sprite_;
}

void ClientEntity::SetInterpolationPosition(
  const b2Vec2& position,
  int64_t snapshot_time,
  int64_t interpolation_offset,
  int64_t server_time
) {
  CHECK(server_time - interpolation_offset < snapshot_time);
  b2Vec2 impulse = GetMass() /
      (snapshot_time - (server_time - interpolation_offset)) * 1000 *
      (b2Vec2(position.x, position.y) - GetPosition());
  SetImpulse(impulse);
}

void ClientEntity::EnableCaption(
  const std::string& caption, const sf::Font& font
) {
  CHECK(caption_visible_ == false);
  caption_text_ = sf::Text(caption, font, 12);
  sf::FloatRect rect = caption_text_.getLocalBounds();
  sf::Vector2f origin(rect.left + rect.width / 2, rect.top  + rect.height / 2);
  caption_text_.setOrigin(Round(origin));
  caption_visible_ = true;
}

void ClientEntity::Render(sf::RenderWindow* render_window, int64_t time) {
  b2Vec2 b2p = GetPosition();
  sf::Vector2f position = Round(sf::Vector2f(b2p.x, b2p.y));

  if (visible_) {
    sprite_->SetPosition(position);
    sprite_->SetRotation(GetRotation() / M_PI * 180.0f);
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
