// Copyright (c) 2015 Blowmorph Team

#ifndef CLIENT_CONTACT_LISTENER_H_
#define CLIENT_CONTACT_LISTENER_H_

#include <Box2D/Box2D.h>

#include "base/pstdint.h"

#include "client/object.h"

namespace bm {

class ContactListener : public b2ContactListener {
  virtual void PreSolve(b2Contact* contact, const b2Manifold* old_manifold) {
    Object* a = static_cast<Object*>(
        contact->GetFixtureA()->GetBody()->GetUserData());
    Object* b = static_cast<Object*>(
        contact->GetFixtureB()->GetBody()->GetUserData());
    if (a->GetType() == Object::TYPE_PLAYER &&
        b->GetType() == Object::TYPE_BULLET) {
      // TODO(xairy): check that bullet belongs to player.
      contact->SetEnabled(false);
    }
  }

  virtual void BeginContact(b2Contact* contact) { }

  virtual void EndContact(b2Contact* contact) { }
};

}  // namespace bm

#endif  // CLIENT_CONTACT_LISTENER_H_
