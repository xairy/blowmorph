// Copyright (c) 2015 Blowmorph Team

#ifndef ENGINE_UTILS_H_
#define ENGINE_UTILS_H_

#include <Box2D/Box2D.h>

namespace bm {

// XXX(xairy): move somewhere?
static const int BOX2D_SCALE = 16;

float Length(const b2Vec2& vector);

b2Body* RayCast(b2World* world, const b2Vec2& start, const b2Vec2& end);

}  // namespace bm

#endif  // ENGINE_UTILS_H_
