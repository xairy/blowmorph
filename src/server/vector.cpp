#include "vector.hpp"

#include <cmath>

namespace bm {

Vector2f::Vector2f(float x, float y) : Vector2<float>(x, y) { }

Vector2f::Vector2f(const Vector2<float>& vector) : Vector2<float>(vector.x, vector.y) { }

float Vector2f::Magnitude() const {
  return sqrt(x * x + y * y);
}

void Vector2f::Normalize() {
  float magnitude = Magnitude();
  if(magnitude != 0.0f) {
    x /= magnitude;
    y /= magnitude;
  }
}

} // namespace bm
