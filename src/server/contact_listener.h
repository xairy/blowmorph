// Copyright (c) 2013 Blowmorph Team

#ifndef SERVER_CONTACT_LISTENER_H_
#define SERVER_CONTACT_LISTENER_H_

#include <Box2D/Box2D.h>

#include "base/pstdint.h"

#include "server/entity.h"

namespace bm {

class ContactListener : public b2ContactListener {
  void BeginContact(b2Contact* contact) {
    Entity* a = static_cast<Entity*>(
        contact->GetFixtureA()->GetBody()->GetUserData());
    Entity* b = static_cast<Entity*>(
        contact->GetFixtureB()->GetBody()->GetUserData());
    a->Collide(b);
  }

  void EndContact(b2Contact* contact) { }
};

}

#endif  // SERVER_CONTACT_LISTENER_H_
