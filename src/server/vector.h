#ifndef BLOWMORPH_SERVER_VECTOR_H_
#define BLOWMORPH_SERVER_VECTOR_H_

namespace bm {

template<class T> struct Vector2 {
  Vector2(T x = T(), T y = T());
  ~Vector2();

  Vector2<T>& operator *= (T scalar);
  Vector2<T>& operator /= (T scalar);

  Vector2<T>& operator += (const Vector2<T>& other);
  Vector2<T>& operator -= (const Vector2<T>& other);

  float x, y;
};

template<class T> Vector2<T> operator * (const Vector2<T>& vector, T scalar);
template<class T> Vector2<T> operator * (T scalar, const Vector2<T>& vector);

template<class T> Vector2<T> operator / (const Vector2<T>& vector, T scalar);

template<class T> Vector2<T> operator + (const Vector2<T>& first, const Vector2<T>& second);
template<class T> Vector2<T> operator - (const Vector2<T>& first, const Vector2<T>& second);

typedef Vector2<float> Vector2f;

float Magnitude(const Vector2f& vector);
Vector2f Normalize(const Vector2f& vector);

} // namespace bm

#include "vector.inl"

#endif // BLOWMORPH_SERVER_VECTOR_H_
