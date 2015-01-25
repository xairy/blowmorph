// Copyright (c) 2015 Blowmorph Team

#include "base/utils.h"

#include <cstdio>

#include <sstream>
#include <string>

#include <Box2D/Box2D.h>

#include "base/macros.h"

namespace bm {

// XXX(xairy): not thread safe because of rand().
size_t Random(size_t max) {
  CHECK(max > 0);
  double zero_to_one = static_cast<double>(rand()) /  // NOLINT
    (static_cast<double>(RAND_MAX) + 1.0f);
  return static_cast<size_t>(zero_to_one * max);
}

std::string IntToStr(int value) {
  std::stringstream ss;
  ss << value;
  return ss.str();
}

float Length(const b2Vec2& vector) {
  return sqrt(vector.x * vector.x + vector.y * vector.y);
}

struct RayCastCallback : public b2RayCastCallback {
  RayCastCallback() : body(NULL) { }

  float ReportFixture(b2Fixture* fixture, const b2Vec2& point,
      const b2Vec2& normal, float fraction) {
    body = fixture->GetBody();
    return fraction;
  }

  b2Body *body;
};

b2Body* RayCast(b2World* world, const b2Vec2& start, const b2Vec2& end) {
  RayCastCallback callback;
  world->RayCast(&callback, 1.0f / 16 * start, 1.0f / 16 * end);
  return callback.body;
}

}  // namespace bm
