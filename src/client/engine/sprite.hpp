#ifndef BLOWMORPH_CLIENT_SPRITE_HPP_
#define BLOWMORPH_CLIENT_SPRITE_HPP_

#include <glm/glm.hpp>
#include <SFML/Graphics.hpp>

namespace bm {

class TextureAtlas;

class Sprite {
public:
  Sprite(bm::TextureAtlas* texture, size_t tile = 0);
  ~Sprite();

  void Render(sf::RenderWindow& render_window);

  void SetPosition(const glm::vec2& value);
  glm::vec2 GetPosition() const;

  // Setter and getter for pivot
  // (a point relative to the texture to which sprite is attached).
  void SetPivot(const glm::vec2& value);
  glm::vec2 GetPivot() const;

private:
  sf::Sprite sprite;
};

} // namespace bm

#endif // BLOWMORPH_CLIENT_SPRITE_HPP_
