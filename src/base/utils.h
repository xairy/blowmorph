// Copyright (c) 2015 Blowmorph Team

#ifndef BASE_UTILS_H_
#define BASE_UTILS_H_

#include <Box2D/Box2D.h>

#include <string>

namespace bm {

static const int BOX2D_SCALE = 16;

// Returns random number in the range [0, max).
size_t Random(size_t max);

std::string IntToStr(int value);

float Length(const b2Vec2& vector);

b2Body* RayCast(b2World* world, const b2Vec2& start, const b2Vec2& end);

}  // namespace bm

#endif  // BASE_UTILS_H_
