#include "vector.hpp"

#include <cmath>

namespace bm {

float Magnitude(const Vector2f& v) {
  return sqrt(v.x * v.x + v.y * v.y);
}

Vector2f Normalize(const Vector2f& vector) {
  Vector2f result(vector);
  float magnitude = Magnitude(result);
  if(magnitude != 0.0f) {
    result.x /= magnitude;
    result.y /= magnitude;
  }
  return result;
}

} // namespace bm
