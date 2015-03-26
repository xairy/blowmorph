// Copyright (c) 2015 Blowmorph Team

#ifndef ENGINE_UTILS_H_
#define ENGINE_UTILS_H_

#include <Box2D/Box2D.h>

#include "engine/dll.h"

namespace bm {

// XXX(xairy): move somewhere?
static const int BOX2D_SCALE = 16;

BM_ENGINE_DECL float Length(const b2Vec2& vector);

BM_ENGINE_DECL b2Body* RayCast(b2World* world, const b2Vec2& start, const b2Vec2& end);

}  // namespace bm

#endif  // ENGINE_UTILS_H_
