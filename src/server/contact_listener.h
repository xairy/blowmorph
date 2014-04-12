// Copyright (c) 2013 Blowmorph Team

#ifndef SERVER_CONTACT_LISTENER_H_
#define SERVER_CONTACT_LISTENER_H_

#include <Box2D/Box2D.h>

#include "base/pstdint.h"

#include "server/entity.h"
#include "server/bullet.h"
#include "server/player.h"

namespace bm {

class ContactListener : public b2ContactListener {
  virtual void PreSolve(b2Contact* contact, const b2Manifold* old_manifold) {
    Entity* a = static_cast<Entity*>(
        contact->GetFixtureA()->GetBody()->GetUserData());
    Entity* b = static_cast<Entity*>(
        contact->GetFixtureB()->GetBody()->GetUserData());
    if (a->GetType() == Entity::TYPE_PLAYER && 
        b->GetType() == Entity::TYPE_BULLET) {
      Player* player = static_cast<Player*>(a);
      Bullet* bullet = static_cast<Bullet*>(b);
      if (player->GetId() == bullet->GetOwnerId()) {
        contact->SetEnabled(false);
      }
    }
  }

  virtual void BeginContact(b2Contact* contact) {
    Entity* a = static_cast<Entity*>(
        contact->GetFixtureA()->GetBody()->GetUserData());
    Entity* b = static_cast<Entity*>(
        contact->GetFixtureB()->GetBody()->GetUserData());
    a->Collide(b);
  }

  virtual void EndContact(b2Contact* contact) { }
};

}

#endif  // SERVER_CONTACT_LISTENER_H_
