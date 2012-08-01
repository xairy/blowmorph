#include "vector.hpp"

#include <cmath>

namespace bm {

Vector2::Vector2(float x, float y) : x(x), y(y) { }
Vector2::~Vector2() { }

float Vector2::Magnitude() const {
  return sqrt(x * x + y * y);
}

Vector2& Vector2::operator *= (float scalar) {
  x *= scalar;
  y *= scalar;
  return *this;
}

Vector2& Vector2::operator /= (float scalar) {
  x /= scalar;
  y /= scalar;
  return *this;
}

Vector2& Vector2::operator += (const Vector2& other) {
  x += other.x;
  y += other.y;
  return *this;
}

Vector2& Vector2::operator -= (const Vector2& other) {
  x -= other.x;
  y -= other.y;
  return *this;
}

Vector2 operator * (const Vector2& vector, float scalar) {
  Vector2 result = vector;
  result *= scalar;
  return result;
}

Vector2 operator * (float scalar, const Vector2& vector) {
  Vector2 result = vector;
  result *= scalar;
  return result;
}

Vector2 operator / (const Vector2& vector, float scalar) {
  Vector2 result = vector;
  result /= scalar;
  return result;
}

Vector2 operator + (const Vector2& first, const Vector2& second) {
  Vector2 result = first;
  result += second;
  return result;
}

Vector2 operator - (const Vector2& first, const Vector2& second) {
  Vector2 result = first;
  result -= second;
  return result;
}

} // namespace bm
