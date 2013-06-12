// XXX[12.8.2012 alex]: better include order?
#define _USE_MATH_DEFINES
#include "sprite.hpp"

#include <cmath>

#include <GL/glew.h>
#include <glm/glm.hpp>

#include <base/macros.hpp>

#include "texture_atlas.hpp"
#include <SFML/Graphics.hpp>

#define _USE_MATH_DEFINES
#include <math.h>
static float fround(float f) {
  return ::floorf(f + 0.5f);
}

namespace bm {

Sprite::Sprite() { }
Sprite::~Sprite() { }

bool Sprite::Init(bm::TextureAtlas* texture, size_t tile) {
  CHECK(texture != NULL);
  CHECK(tile < texture->GetTileCount());

  glm::vec2 tile_position = texture->GetTilePosition(tile);
  glm::vec2 tile_size = texture->GetTileSize(tile);

  sprite.setTexture(*texture->GetTexture());
  sprite.setTextureRect(sf::IntRect(tile_position.x, tile_position.y,
    tile_size.x, tile_size.y));

  return true;
}

// TODO[xairy]: render sprite from outside.
void Sprite::Render(sf::RenderWindow& render_window) {
  render_window.draw(sprite);
}

void Sprite::SetPosition(const glm::vec2& position) {
  sprite.setPosition(sf::Vector2f(position.x, position.y));
}
glm::vec2 Sprite::GetPosition() const {
  return glm::vec2(sprite.getPosition().x, sprite.getPosition().y);
}

void Sprite::SetPivot(const glm::vec2& pivot) {
  sprite.setOrigin(pivot.x, pivot.y);
}
glm::vec2 Sprite::GetPivot() const {
  return glm::vec2(sprite.getOrigin().x, sprite.getOrigin().y);
}

} // namespace bm
