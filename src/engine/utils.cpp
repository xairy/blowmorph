// Copyright (c) 2015 Blowmorph Team

#include "engine/utils.h"

#include <Box2D/Box2D.h>

#include "base/macros.h"

namespace bm {

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
