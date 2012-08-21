#ifndef BLOWMORPH_SERVER_VECTOR_HPP_
#define BLOWMORPH_SERVER_VECTOR_HPP_

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

class Vector2f : public Vector2<float> {
public:
  Vector2f(float x = 0.0f, float y = 0.0f);
  Vector2f(const Vector2<float>& vector);

  float Magnitude() const;
  void Normalize();

  // XXX[21.08.2012 xairy].
  // Vector2f a, b;
  // float f = (a - b).Magnitude(); // Does not work.
  // float f = Vector2f(a - b).Magnitude(); // Works.
};

} // namespace bm

#include "vector.inl"

#endif // BLOWMORPH_SERVER_VECTOR_HPP_
