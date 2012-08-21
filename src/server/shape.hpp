#ifndef BLOWMORPH_SERVER_SHAPE_HPP_
#define BLOWMORPH_SERVER_SHAPE_HPP_

#include "vector.hpp"

namespace bm {

class Circle;
class Rectangle;

class Shape {
public:
  Shape(const Vector2f& position);
  virtual ~Shape();

  virtual Vector2f GetPosition() const;
  virtual void SetPosition(const Vector2f& position);

  virtual void Move(const Vector2f& value);

  virtual bool Collide(Shape* shape) = 0;

  virtual bool Collide(Circle* circle) = 0;
  virtual bool Collide(Rectangle* circle) = 0;

  static bool Collide(Circle* circle1, Circle* circle2);
  static bool Collide(Circle* circle, Rectangle* rectangle);
  static bool Collide(Rectangle* rectangle1, Rectangle* rectangle2);

protected:
  Vector2f _position;
};

class Circle : public Shape {
  //friend class Shape;

public:
  Circle(const Vector2f& center, float radius);
  ~Circle();

  Vector2f GetCenter() const;
  float GetRadius() const;

  void SetCenter(const Vector2f& center);
  void SetRadius(float radius);

  virtual bool Collide(Shape* shape);

  virtual bool Collide(Circle* circle);
  virtual bool Collide(Rectangle* rectangle);

protected:
  float _radius;
};

class Rectangle : public Shape {
  //friend class Shape;

public:
  Rectangle(const Vector2f& center, float width, float height);
  virtual ~Rectangle();

  Vector2f GetCenter() const;
  float GetWidth() const;
  float GetHeight() const;

  void SetCenter(const Vector2f& center);
  void SetWidth(float width);
  void SetHeight(float height);

  virtual bool Collide(Shape* shape);

  virtual bool Collide(Circle* circle);
  virtual bool Collide(Rectangle* rectangle);

protected:
  float _width;
  float _height;
};

class Square : public Rectangle {
public:
  Square(const Vector2f& center, float side);
  ~Square();
};

};

#endif // BLOWMORPH_SERVER_SHAPE_HPP_
