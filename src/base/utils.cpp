// Copyright (c) 2015 Blowmorph Team

#include "base/utils.h"

#include <cstdio>

#include <Box2D/Box2D.h>

namespace bm {

struct RayCastCallback : public b2RayCastCallback {
	RayCastCallback() : body(NULL) { }

	float ReportFixture(b2Fixture* fixture, const b2Vec2& point,
			const b2Vec2& normal, float fraction) {
		body = fixture->GetBody();
		return fraction;
	}

	b2Body *body;
};

b2Body* RayCast(b2World* world, const b2Vec2& start, const b2Vec2& end) {
	RayCastCallback callback;
	world->RayCast(&callback, 1.0f / 16 * start, 1.0f / 16 * end);
	return callback.body;
}

}  // namespace bm
