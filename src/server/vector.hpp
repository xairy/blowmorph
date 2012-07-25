#ifndef BLOWMORPH_SERVER_VECTOR_HPP_
#define BLOWMORPH_SERVER_VECTOR_HPP_

#include <cmath>

namespace bm {

struct Vector2 {
  Vector2(float x = 0.0f, float y = 0.0f) : x(x), y(y) { }
  ~Vector2() { }

  float Magnitude() const {
    return sqrt(x * x + y * y);
  }

  Vector2& operator *= (float scalar) {
    x *= scalar;
    y *= scalar;
    return *this;
  }

  Vector2& operator /= (float scalar) {
    x /= scalar;
    y /= scalar;
    return *this;
  }

  Vector2& operator += (const Vector2& other) {
    x += other.x;
    y += other.y;
    return *this;
  }

  Vector2& operator -= (const Vector2& other) {
    x -= other.x;
    y -= other.y;
    return *this;
  }

  float x, y;
};

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

#endif // BLOWMORPH_SERVER_VECTOR_HPP_
