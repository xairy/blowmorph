#ifndef BLOWMORPH_SERVER_SHAPE_HPP_
#define BLOWMORPH_SERVER_SHAPE_HPP_

#include "vector.hpp"

namespace bm {

class Circle;
class Rectangle;

class Shape {
public:
  Shape(const Vector2& position) : _position(position) { }
  virtual ~Shape() { }

  virtual Vector2 GetPosition() const {
    return _position;
  }
  virtual void SetPosition(const Vector2& position) {
    _position = position;
  }

  virtual void Move(const Vector2& value) {
    _position += value;
  }

  virtual bool Collide(Shape* shape) = 0;

  virtual bool Collide(Circle* circle) = 0;
  virtual bool Collide(Rectangle* circle) = 0;

  static bool Collide(Circle* circle1, Circle* circle2);
  static bool Collide(Circle* circle, Rectangle* rectangle);
  static bool Collide(Rectangle* rectangle1, Rectangle* rectangle2);

protected:
  Vector2 _position;
};

class Circle : public Shape {
  //friend class Shape;

public:
  Circle(const Vector2& center, float radius)
    : Shape(center), _radius(radius) { }
  ~Circle() { }

  Vector2 GetCenter() const {
    return _position;
  }
  float GetRadius() const {
    return _radius;
  }

  void SetCenter(const Vector2& center) {
    _position = center;
  }
  void SetRadius(float radius) {
    _radius = radius;
  }

  virtual bool Collide(Shape* shape) {
    return shape->Collide(this);
  }

  virtual bool Collide(Circle* circle) {
    return Shape::Collide(this, circle);
  }
  virtual bool Collide(Rectangle* rectangle) {
    return Shape::Collide(this, rectangle);
  }

protected:
  float _radius;
};

class Rectangle : public Shape {
  //friend class Shape;

public:
  Rectangle(const Vector2& center, float width, float height)
    : Shape(center), _width(width), _height(height) { }
  virtual ~Rectangle() { }

  Vector2 GetCenter() const {
    return _position;
  }
  float GetWidth() const {
    return _width;
  }
  float GetHeight() const {
    return _height;
  }

  void SetCenter(const Vector2& center) {
    _position = center;
  }
  void SetWidth(float width) {
    _width = width;
  }
  void SetHeight(float height) {
    _height = height;
  }

  virtual bool Collide(Shape* shape) {
    return shape->Collide(this);
  }

  virtual bool Collide(Circle* circle) {
    return Shape::Collide(circle, this);
  }
  virtual bool Collide(Rectangle* rectangle) {
    return Shape::Collide(this, rectangle);
  }

protected:
  float _width;
  float _height;
};

class Square : public Rectangle {
public:
  Square(const Vector2& center, float side)
    : Rectangle(center, side, side) { }
  ~Square() { }
};

bool Shape::Collide(Circle* circle1, Circle* circle2) {
  float dx = circle1->GetCenter().x - circle2->GetCenter().x;
  float dy = circle1->GetCenter().y - circle2->GetCenter().y;
  float distance2 = dx * dx + dy * dy;
  float allowed_distance = circle1->GetRadius() + circle2->GetRadius();
  float allowed_distance2 = allowed_distance * allowed_distance;
  return distance2 <= allowed_distance2;
}

bool Shape::Collide(Circle* circle, Rectangle* rectangle) {
  float cx = circle->GetCenter().x;
  float cy = circle->GetCenter().y;
  float r = circle->GetRadius();

  float rx = rectangle->GetCenter().x;
  float ry = rectangle->GetCenter().y;
  float w = rectangle->GetWidth();
  float h = rectangle->GetHeight();

  float dx = abs(cx - rx);
  float dy = abs(cy - ry);

  if(dx > w / 2 + r || dy > h / 2 + r) {
    return false;
  }
  if(dx <= w / 2 || dy <= h / 2) {
    return true;
  }
  float cd2 = (dx - w / 2) * (dx - w / 2) + (dy - h / 2) * (dy - h / 2);
  return cd2 <= r * r;
}

bool Shape::Collide(Rectangle* rectangle1, Rectangle* rectangle2) {
  float dx = abs(rectangle1->GetCenter().x - rectangle2->GetCenter().x);
  float dy = abs(rectangle1->GetCenter().y - rectangle2->GetCenter().y);
  float allowed_x = (rectangle1->GetWidth() + rectangle2->GetWidth()) / 2;
  float allowed_y = (rectangle1->GetHeight() + rectangle2->GetHeight()) / 2;
  return (dx <= allowed_x && dy <= allowed_y);
}

};

#endif // BLOWMORPH_SERVER_SHAPE_HPP_
