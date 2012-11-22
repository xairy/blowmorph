#ifndef BLOWMORPH_CLIENT_CANVAS_HPP_
#define BLOWMORPH_CLIENT_CANVAS_HPP_

#define _USE_MATH_DEFINES
#include <glm/glm.hpp>

namespace bm {

class TextureAtlas;

class Canvas {
public:
  enum CoordinateType {
    Normalized,
    Pixels,
    NormalizedFlipped,
    PixelsFlipped
  };
  
  bool Init();

  glm::vec2 GetSize();
  void SetCoordinateType(CoordinateType ct);
  void SetTransform(const glm::mat3x3& m);
  
  void DrawRect(const glm::vec4& clr, const glm::vec2& pos, const glm::vec2& size);
  void FillRect(const glm::vec4& clr, const glm::vec2& pos, const glm::vec2& size);
  void DrawCircle(const glm::vec4& clr, const glm::vec2& pos, float radius, size_t steps);
  void FillCircle(const glm::vec4& clr, const glm::vec2& pos, float radius, size_t steps);
  
  void DrawTexturedQuad(const glm::vec2& pos, const glm::vec2& pivot, glm::float_t zIndex, 
                        TextureAtlas* texture, size_t tile);

private:
  
};

}; // namespace bm

#endif//BLOWMORPH_CLIENT_CANVAS_HPP_