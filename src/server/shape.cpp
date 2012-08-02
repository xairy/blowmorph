#include "shape.hpp"

#include <cmath>
#include <cstdlib>

#include "vector.hpp"

namespace bm {

Shape::Shape(const Vector2& position) : _position(position) { }
Shape::~Shape() { }

Vector2 Shape::GetPosition() const {
  return _position;
}
void Shape::SetPosition(const Vector2& position) {
  _position = position;
}

void Shape::Move(const Vector2& value) {
  _position += value;
}


Circle::Circle(const Vector2& center, float radius)
  : Shape(center), _radius(radius) { }
Circle::~Circle() { }

Vector2 Circle::GetCenter() const {
  return _position;
}
float Circle::GetRadius() const {
  return _radius;
}

void Circle::SetCenter(const Vector2& center) {
  _position = center;
}
void Circle::SetRadius(float radius) {
  _radius = radius;
}

bool Circle::Collide(Shape* shape) {
  return shape->Collide(this);
}

bool Circle::Collide(Circle* circle) {
  return Shape::Collide(this, circle);
}
bool Circle::Collide(Rectangle* rectangle) {
  return Shape::Collide(this, rectangle);
}


Rectangle::Rectangle(const Vector2& center, float width, float height)
  : Shape(center), _width(width), _height(height) { }
Rectangle::~Rectangle() { }

Vector2 Rectangle::GetCenter() const {
  return _position;
}
float Rectangle::GetWidth() const {
  return _width;
}
float Rectangle::GetHeight() const {
  return _height;
}

void Rectangle::SetCenter(const Vector2& center) {
  _position = center;
}
void Rectangle::SetWidth(float width) {
  _width = width;
}
void Rectangle::SetHeight(float height) {
  _height = height;
}

bool Rectangle::Collide(Shape* shape) {
  return shape->Collide(this);
}

bool Rectangle::Collide(Circle* circle) {
  return Shape::Collide(circle, this);
}
bool Rectangle::Collide(Rectangle* rectangle) {
  return Shape::Collide(this, rectangle);
}


Square::Square(const Vector2& center, float side)
  : Rectangle(center, side, side) { }
Square::~Square() { }


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
  return (dx < allowed_x && dy < allowed_y);
}

} // namespace bm
