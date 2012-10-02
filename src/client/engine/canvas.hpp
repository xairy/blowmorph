#ifndef BLOWMORPH_CLIENT_CANVAS_HPP_
#define BLOWMORPH_CLIENT_CANVAS_HPP_

#define _USE_MATH_DEFINES
#include <glm/glm.hpp>

namespace bm {

class Canvas {
public:
  enum CoordinateType {
    Normalized,
    Pixels,
    NormalizedFlipped,
    PixelsFlipped
  };
  
  bool Init();

  void SetCoordinateType(CoordinateType ct);
  void SetTransform(const glm::mat3x3& m);
  
  void DrawRect(const glm::vec4& clr, const glm::vec2& pos, const glm::vec2& size);
  void FillRect(const glm::vec4& clr, const glm::vec2& pos, const glm::vec2& size);
  void DrawCircle(const glm::vec4& clr, const glm::vec2& pos, float radius, size_t steps);
  void FillCircle(const glm::vec4& clr, const glm::vec2& pos, float radius, size_t steps);

private:
  
};

}; // namespace bm

#endif//BLOWMORPH_CLIENT_CANVAS_HPP_