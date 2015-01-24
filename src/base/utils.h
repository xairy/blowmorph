// Copyright (c) 2015 Blowmorph Team

#ifndef BASE_UTILS_H_
#define BASE_UTILS_H_

#include <Box2D/Box2D.h>

namespace bm {

static const int BOX2D_SCALE = 16;

b2Body* RayCast(b2World* world, const b2Vec2& start, const b2Vec2& end);

}  // namespace bm

#endif  // BASE_UTILS_H_
