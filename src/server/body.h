// Copyright (c) 2013 Blowmorph Team

#ifndef SERVER_BODY_H_
#define SERVER_BODY_H_

#include <string>

#include <Box2D/Box2D.h>

#include "base/macros.h"
#include "base/settings_manager.h"

namespace bm {

class Body {
 public:
  static const int BOX2D_SCALE = 16;

 public:
  Body();
  virtual ~Body();

  void Create(b2World* world, SettingsManager* settings,
      const std::string& prefix, bool dynamic);
  void Destroy();

  b2Body* GetBody();

  b2Vec2 GetPosition() const;
  void SetPosition(const b2Vec2& position);

  b2Vec2 GetVelocity() const;
  void SetVelocity(const b2Vec2& velocity);

  void SetUserData(void* data);
  void SetCollisionFilter(int16_t category, int16_t mask);

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

#endif  // SERVER_BODY_H_