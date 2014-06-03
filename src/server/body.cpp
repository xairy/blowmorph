// Copyright (c) 2013 Blowmorph Team

#include "server/body.h"

#include <string>

#include <Box2D/Box2D.h>

#include "base/macros.h"
#include "base/settings_manager.h"

namespace bm {

Body::Body() : world_(NULL), body_(NULL), state_(STATE_DESTROYED) { }

Body::~Body() {
  if (state_ == STATE_CREATED) {
    Destroy();
  }
}

void Body::Create(b2World* world, SettingsManager* settings,
    const std::string& prefix, bool dynamic) {
  CHECK(state_ == STATE_DESTROYED);

  CHECK(world != NULL);
  CHECK(settings != NULL);

  world_ = world;

  b2BodyDef body_def;
  body_def.type = dynamic ? b2_dynamicBody : b2_staticBody;
  body_def.fixedRotation = true;
  body_ = world_->CreateBody(&body_def);
  CHECK(body_ != NULL);

  b2FixtureDef fixture_def;
  fixture_def.density = 1.0f;
  fixture_def.friction = 0.0f;

  std::string type = settings->GetString(prefix + ".type");
  if (type == "box") {
    float width = settings->GetFloat(prefix + ".width");
    float height = settings->GetFloat(prefix + ".height");
    b2PolygonShape shape;
    shape.SetAsBox(width / 2 / BOX2D_SCALE, height / 2 / BOX2D_SCALE);
    fixture_def.shape = &shape;
    body_->CreateFixture(&fixture_def);
  } else if (type == "circle") {
    float radius = settings->GetFloat(prefix + ".radius");
    b2CircleShape shape;
    shape.m_radius = radius / BOX2D_SCALE;
    fixture_def.shape = &shape;
    body_->CreateFixture(&fixture_def);
  } else {
    CHECK(false);  // Incorrect shape type.
  }

  state_ = STATE_CREATED;
}

void Body::Destroy() {
  CHECK(state_ == STATE_CREATED);
  world_->DestroyBody(body_);
  world_ = NULL;
  body_ = NULL;
  state_ = STATE_DESTROYED;
}

b2Body* Body::GetBody() {
  CHECK(state_ == STATE_CREATED);
  return body_;
}

b2Vec2 Body::GetPosition() const {
  CHECK(state_ == STATE_CREATED);
  b2Vec2 position = body_->GetPosition();
  position *= BOX2D_SCALE;
  return position;
}

void Body::SetPosition(const b2Vec2& position) {
  CHECK(state_ == STATE_CREATED);
  b2Vec2 scaled_position = position;
  scaled_position *= 1.0f / BOX2D_SCALE;
  body_->SetTransform(scaled_position, body_->GetAngle());
}

b2Vec2 Body::GetVelocity() const {
  CHECK(state_ == STATE_CREATED);
  b2Vec2 velocity = body_->GetLinearVelocity();
  velocity *= BOX2D_SCALE;
  return velocity;
}

void Body::SetVelocity(const b2Vec2& velocity) {
  CHECK(state_ == STATE_CREATED);
  b2Vec2 scaled_velocity = velocity;
  scaled_velocity *= 1.0f / BOX2D_SCALE;
  body_->SetLinearVelocity(scaled_velocity);
}

void Body::SetUserData(void* data) {
  CHECK(state_ == STATE_CREATED);
  body_->SetUserData(data);
}

// XXX(xairy): works with only one fixture per body.
void Body::SetCollisionFilter(int16_t category, int16_t mask) {
  CHECK(state_ == STATE_CREATED);
  b2Fixture* fixture = body_->GetFixtureList();
  b2Filter filter;
  filter.categoryBits = category;
  filter.maskBits = mask;
  fixture->SetFilterData(filter);
}

}  // namespace bm
