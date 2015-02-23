// Copyright (c) 2015 Blowmorph Team

#ifndef CLIENT_ENTITY_H_
#define CLIENT_ENTITY_H_

#include <cmath>

#include <string>

#include <SFML/Graphics.hpp>

#include "base/pstdint.h"

#include "engine/body.h"
#include "engine/entity.h"

#include "client/sprite.h"

namespace bm {

class ClientEntity : public Entity {
 public:
  ClientEntity(
    b2World* world,
    uint32_t id,
    Type type,
    const std::string& entity_name,
    b2Vec2 position,
    Sprite* sprite);
  ~ClientEntity();

  Sprite* GetSprite();

  bool HasCaption();
  sf::Text* GetCaption();

  // Makes object to start moving towards 'position' to be there when
  // current time - 'interpolation_offset' == 'time'.
  void SetInterpolationPosition(const b2Vec2& position,
      int64_t snapshot_time, int64_t interpolation_offset, int64_t server_time);

  void EnableCaption(const std::string& caption, const sf::Font& font);

 private:
  Sprite* sprite_;

  bool caption_visible_;
  sf::Text caption_text_;
};

}  // namespace bm

#endif  // CLIENT_ENTITY_H_
