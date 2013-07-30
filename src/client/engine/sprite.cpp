#include "sprite.hpp"

#include <base/macros.hpp>

#include "texture_atlas.hpp"

#include <SFML/Graphics.hpp>

namespace bm {

Sprite::Sprite(bm::TextureAtlas* texture, size_t tile) {
  CHECK(texture != NULL);
  CHECK(tile < texture->GetTileCount());

  glm::vec2 tile_position = texture->GetTilePosition(tile);
  glm::vec2 tile_size = texture->GetTileSize(tile);

  sprite.setTexture(*texture->GetTexture());
  sprite.setTextureRect(sf::IntRect(tile_position.x, tile_position.y,
    tile_size.x, tile_size.y));

  // XXX[xairy]: pivot is set by default.
  SetPivot(glm::vec2(tile_size.x / 2.0f, tile_size.y / 2.0f));
}

Sprite::~Sprite() { }

// TODO[xairy]: render sprite from outside?
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
