// Copyright (c) 2013 Blowmorph Team

#ifndef SERVER_CONTACT_LISTENER_H_
#define SERVER_CONTACT_LISTENER_H_

#include <Box2D/Box2D.h>

#include "base/pstdint.h"

#include "server/entity.h"
#include "server/projectile.h"
#include "server/player.h"

namespace bm {

class ContactListener : public b2ContactListener {
  virtual void PreSolve(b2Contact* contact, const b2Manifold* old_manifold) {
    Entity* a = static_cast<Entity*>(
        contact->GetFixtureA()->GetBody()->GetUserData());
    Entity* b = static_cast<Entity*>(
        contact->GetFixtureB()->GetBody()->GetUserData());
    if (a->GetType() == Entity::TYPE_PLAYER &&
        b->GetType() == Entity::TYPE_PROJECTILE) {
      Player* player = static_cast<Player*>(a);
      Projectile* projectile = static_cast<Projectile*>(b);
      if (player->GetId() == projectile->GetOwnerId()) {
        contact->SetEnabled(false);
      }
    }
    if (b->GetType() == Entity::TYPE_PLAYER &&
        a->GetType() == Entity::TYPE_PROJECTILE) {
      Player* player = static_cast<Player*>(b);
      Projectile* projectile = static_cast<Projectile*>(a);
      if (player->GetId() == projectile->GetOwnerId()) {
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

}  // namespace bm

#endif  // SERVER_CONTACT_LISTENER_H_
