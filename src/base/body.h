// Copyright (c) 2013 Blowmorph Team

#ifndef BASE_BODY_H_
#define BASE_BODY_H_

#include <string>

#include <Box2D/Box2D.h>

#include "base/macros.h"
#include "base/settings_manager.h"
#include "base/utils.h"

namespace bm {

class Body {
 public:
  Body();
  virtual ~Body();

  void Create(b2World* world, SettingsManager* body_settings,
      const std::string& body_config);
  void Destroy();

  b2Body* GetBody();

  b2Vec2 GetPosition() const;
  void SetPosition(const b2Vec2& position);

  float GetRotation() const;
  void SetRotation(float angle);

  b2Vec2 GetVelocity() const;
  void SetVelocity(const b2Vec2& velocity);

  float GetMass() const;

  void ApplyImpulse(const b2Vec2& impulse);
  void SetImpulse(const b2Vec2& impulse);

  void SetUserData(void* data);
  void SetCollisionFilter(int16_t category, int16_t mask);
  void SetType(b2BodyType type);

 private:
  b2World* world_;
  b2Body* body_;

  enum {
    STATE_DESTROYED,
    STATE_CREATED
  } state_;

  DISALLOW_COPY_AND_ASSIGN(Body);
};

}  // namespace bm

#endif  // BASE_BODY_H_
