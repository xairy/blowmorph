// Copyright (c) 2013 Blowmorph Team

#ifndef SERVER_BOX2D_UTILS_H_
#define SERVER_BOX2D_UTILS_H_

#include <Box2D/Box2D.h>

#include <string>

#include "base/settings_manager.h"

namespace bm {

b2Body* CreateBody(b2World* world, SettingsManager* settings,
    const std::string& prefix, bool dynamic);

void SetBodyPosition(b2Body* body, const b2Vec2& position);

void SetCollisionFilter(b2Body* body, int16_t category, int16_t mask);

}  // namespace bm

#endif  // SERVER_BOX2D_UTILS_H_
