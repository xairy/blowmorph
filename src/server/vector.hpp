#ifndef BLOWMORPH_SERVER_VECTOR_HPP_
#define BLOWMORPH_SERVER_VECTOR_HPP_

namespace bm {

struct Vector2 {
  Vector2(float x = 0.0f, float y = 0.0f);
  ~Vector2();

  float Magnitude() const;

  Vector2& operator *= (float scalar);
  Vector2& operator /= (float scalar);

  Vector2& operator += (const Vector2& other);
  Vector2& operator -= (const Vector2& other);

  float x, y;
};

Vector2 operator * (const Vector2& vector, float scalar);
Vector2 operator * (float scalar, const Vector2& vector);

Vector2 operator / (const Vector2& vector, float scalar);

Vector2 operator + (const Vector2& first, const Vector2& second);
Vector2 operator - (const Vector2& first, const Vector2& second);

} // namespace bm

#endif // BLOWMORPH_SERVER_VECTOR_HPP_
