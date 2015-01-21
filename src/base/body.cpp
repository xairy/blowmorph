// Copyright (c) 2013 Blowmorph Team

#include "base/body.h"

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

void Body::Create(b2World* world, SettingsManager* body_settings,
    const std::string& body_config) {
  CHECK(state_ == STATE_DESTROYED);

  CHECK(world != NULL);
  CHECK(body_settings != NULL);

  world_ = world;

  bool dynamic = body_settings->GetBool(body_config + ".dynamic");
  b2BodyDef body_def;
  body_def.type = dynamic ? b2_dynamicBody : b2_staticBody;
  body_def.fixedRotation = true;
  body_ = world_->CreateBody(&body_def);
  CHECK(body_ != NULL);

  b2FixtureDef fixture_def;
  fixture_def.density = 1.0f;
  fixture_def.friction = 0.0f;
  fixture_def.restitution = 0.0f;

  std::string type = body_settings->GetString(body_config + ".shape.type");
  if (type == "box") {
    float width = body_settings->GetFloat(body_config + ".shape.width");
    float height = body_settings->GetFloat(body_config + ".shape.height");
    b2PolygonShape shape;
    shape.SetAsBox(width / 2 / BOX2D_SCALE, height / 2 / BOX2D_SCALE);
    fixture_def.shape = &shape;
    body_->CreateFixture(&fixture_def);
  } else if (type == "circle") {
    float radius = body_settings->GetFloat(body_config + ".shape.radius");
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

float Body::GetRotation() const {
  CHECK(state_ == STATE_CREATED);
  return body_->GetAngle();
}

void Body::SetRotation(float angle) {
  CHECK(state_ == STATE_CREATED);
  body_->SetTransform(body_->GetPosition(), angle);
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

float Body::GetMass() const {
  CHECK(state_ == STATE_CREATED);
  return body_->GetMass();
}

void Body::ApplyImpulse(const b2Vec2& impulse) {
  CHECK(state_ == STATE_CREATED);
  b2Vec2 scaled_impulse = impulse;
  scaled_impulse *= 1.0f / BOX2D_SCALE;
  body_->ApplyLinearImpulse(scaled_impulse, body_->GetWorldCenter(), true);
}

void Body::SetImpulse(const b2Vec2& impulse) {
  CHECK(state_ == STATE_CREATED);
  b2Vec2 velocity_impulse = -GetMass() * GetVelocity();
  b2Vec2 scaled_impulse = impulse + velocity_impulse;
  scaled_impulse *= 1.0f / BOX2D_SCALE;
  body_->ApplyLinearImpulse(scaled_impulse, body_->GetWorldCenter(), true);
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

void Body::SetType(b2BodyType type) {
  CHECK(state_ == STATE_CREATED);
  body_->SetType(type);
}

}  // namespace bm
