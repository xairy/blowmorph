// Copyright (c) 2013 Blowmorph Team

#include "server/box2d_utils.h"

#include <Box2D/Box2D.h>

#include "base/macros.h"
#include "base/settings_manager.h"

namespace bm {

b2Body* CreateBody(b2World* world, SettingsManager* settings,
    const std::string& prefix, bool dynamic) {
  CHECK(world != NULL);
  CHECK(settings != NULL);

  b2BodyDef body_def;
  body_def.type = dynamic ? b2_dynamicBody : b2_staticBody;
  body_def.fixedRotation = true;
  b2Body* body = world->CreateBody(&body_def);
  CHECK(body != NULL);

  b2FixtureDef fixture_def;
  fixture_def.density = 1.0f;
  fixture_def.friction = 0.0f;

  std::string type = settings->GetString(prefix + ".type");
  if (type == "box") {
    float width = settings->GetFloat(prefix + ".width");
    float height = settings->GetFloat(prefix + ".height");
    b2PolygonShape shape;
    shape.SetAsBox(width / 2, height / 2);
    fixture_def.shape = &shape;
    body->CreateFixture(&fixture_def);
  } else if (type == "circle") {
    float radius = settings->GetFloat(prefix + ".radius");
    b2CircleShape shape;
    shape.m_radius = radius;
    fixture_def.shape = &shape;
    body->CreateFixture(&fixture_def);
  } else {
    CHECK(false);  // Incorrect shape type.
  }

  return body;
}

void SetBodyPosition(b2Body* body, const b2Vec2& position) {
  body->SetTransform(position, body->GetAngle());
}

}
