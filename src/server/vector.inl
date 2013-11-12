// Copyright (c) 2013 Blowmorph Team

#include <cmath>

namespace bm {

template<class T>
Vector2<T>::Vector2(T x, T y) : x(x), y(y) { }

template<class T>
Vector2<T>::~Vector2() { }

template<class T>
Vector2<T>& Vector2<T>::operator *= (T scalar) {
  x *= scalar;
  y *= scalar;
  return *this;
}

template<class T>
Vector2<T>& Vector2<T>::operator /= (T scalar) {
  x /= scalar;
  y /= scalar;
  return *this;
}

template<class T>
Vector2<T>& Vector2<T>::operator += (const Vector2<T>& other) {
  x += other.x;
  y += other.y;
  return *this;
}

template<class T>
Vector2<T>& Vector2<T>::operator -= (const Vector2<T>& other) {
  x -= other.x;
  y -= other.y;
  return *this;
}

template<class T>
Vector2<T> operator * (const Vector2<T>& vector, T scalar) {
  Vector2<T> result = vector;
  result *= scalar;
  return result;
}

template<class T>
Vector2<T> operator * (T scalar, const Vector2<T>& vector) {
  Vector2<T> result = vector;
  result *= scalar;
  return result;
}

template<class T>
Vector2<T> operator / (const Vector2<T>& vector, T scalar) {
  Vector2<T> result = vector;
  result /= scalar;
  return result;
}

template<class T>
Vector2<T> operator + (const Vector2<T>& first, const Vector2<T>& second) {
  Vector2<T> result = first;
  result += second;
  return result;
}

template<class T>
Vector2<T> operator - (const Vector2<T>& first, const Vector2<T>& second) {
  Vector2<T> result = first;
  result -= second;
  return result;
}

}  // namespace bm
