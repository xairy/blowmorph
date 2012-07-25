#ifndef BLOWMORPH_SPRITE_HPP_
#define BLOWMORPH_SPRITE_HPP_

#include <glm/glm.hpp>

namespace bm {

class Texture;

class Sprite {
public:
  Sprite();
  ~Sprite();
 
  bool Init(bm::Texture* texture, size_t tileIndex);
  
  // Renders sprite using OpenGL API.
  void Render();
  
  // Setter and getter for z index.
  void SetZIndex(glm::float_t value);
  glm::float_t GetZIndex() const;
  
  // Setter and getter for position
  void SetPosition(const glm::vec2& value);
  glm::vec2 GetPosition() const;
  
  // Setter and getter for pivot 
  // (a point relative to the texture to which sprite is attached)
  void SetPivot(const glm::vec2& value);
  glm::vec2 GetPivot() const;
  
  // Setter and getter for scale
  void SetScale(const glm::vec2& value);
  glm::vec2 GetScale() const;
  
  // Setter and getter for angle (i.e. rotation of a sprite)
  void SetAngle(glm::float_t value);
  glm::float_t GetAngle() const;
  
  // Getter for sprite size
  glm::uvec2 GetSize() const;
  
  // Rotates sprite by a given number of radians. 
  void Rotate(glm::float_t value);
  
  // Moves sprite.
  void Move(const glm::vec2& value);
  
private:
  glm::float_t zIndex;
  glm::vec2 position;
  glm::vec2 pivot;
  glm::vec2 scale;
  glm::float_t angle;
  
  bm::Texture* texture;
  size_t tileIndex;
};

}; // namespace bm

#endif /*BLOWMORPH_SPRITE_HPP_*/