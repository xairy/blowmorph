// Copyright (c) 2015 Blowmorph Team

#ifndef ENGINE_ENTITY_H_
#define ENGINE_ENTITY_H_

#include <string>

#include <Box2D/Box2D.h>

#include "base/macros.h"
#include "base/pstdint.h"

#include "engine/body.h"
#include "engine/dll.h"
#include "engine/protocol.h"

namespace bm {

class Entity {
 public:
  enum Type {
    TYPE_ACTIVATOR,
    TYPE_CRITTER,
    TYPE_DOOR,
    TYPE_KIT,
    TYPE_PLAYER,
    TYPE_PROJECTILE,
    TYPE_WALL,
  };

  // Collision filters.
  enum FilterType {
    FILTER_DEFAULT    = 0x0001,
    FILTER_ACTIVATOR  = 0x0002,
    FILTER_CRITTER    = 0x0004,
    FILTER_DOOR       = 0x0008,
    FILTER_KIT        = 0x0010,
    FILTER_PLAYER     = 0x0020,
    FILTER_PROJECTILE = 0x0040,
    FILTER_WALL       = 0x0080,
    FILTER_ALL        = 0xffff,
    FILTER_NONE       = 0x0000
  };

 public:
  BM_ENGINE_DECL Entity(
    b2World* world,
    uint32_t id,
    Type type,
    const std::string& entity_name,
    b2Vec2 position,
    uint16_t collision_category,
    uint16_t collision_mask);
  BM_ENGINE_DECL virtual ~Entity();

  BM_ENGINE_DECL uint32_t GetId() const;
  BM_ENGINE_DECL Type GetType() const;
  BM_ENGINE_DECL bool IsStatic() const;

  BM_ENGINE_DECL b2Vec2 GetPosition() const;
  BM_ENGINE_DECL void SetPosition(const b2Vec2& position);

  BM_ENGINE_DECL float GetRotation() const;
  BM_ENGINE_DECL void SetRotation(float angle);

  BM_ENGINE_DECL b2Vec2 GetVelocity() const;
  BM_ENGINE_DECL void SetVelocity(const b2Vec2& velocity);

  BM_ENGINE_DECL float GetMass() const;
  BM_ENGINE_DECL void ApplyImpulse(const b2Vec2& impulse);
  BM_ENGINE_DECL void SetImpulse(const b2Vec2& impulse);

 protected:
  uint32_t id_;
  Type type_;
  Body* body_;
  std::string name_;
};

}  // namespace bm

#endif  // ENGINE_ENTITY_H_
