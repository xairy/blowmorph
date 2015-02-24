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
#include "client/utils.h"

namespace bm {

ClientEntity::ClientEntity(
  b2World* world,
  uint32_t id,
  Type type,
  const std::string& entity_name,
  b2Vec2 position,
  Sprite* sprite
) : Entity(world, id, type, entity_name, position, FILTER_DEFAULT, FILTER_ALL),
    sprite_(sprite),
    caption_visible_(false) {
  // XXX(xairy): create Sprite here?
}

ClientEntity::~ClientEntity() {
  if (sprite_ != NULL) delete sprite_;
}

Sprite* ClientEntity::GetSprite() {
  return sprite_;
}

bool ClientEntity::HasCaption() {
  return caption_visible_;
}

sf::Text* ClientEntity::GetCaption() {
  CHECK(caption_visible_ == true);
  return &caption_text_;
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

}  // namespace bm
