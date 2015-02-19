// Copyright (c) 2015 Blowmorph Team

#ifndef CLIENT_CONTACT_LISTENER_H_
#define CLIENT_CONTACT_LISTENER_H_

#include <Box2D/Box2D.h>

#include "base/pstdint.h"

#include "client/entity.h"

namespace bm {

class ContactListener : public b2ContactListener {
 public:
  ContactListener() : player_id_set_(false), player_id_(0) { }

  void SetPlayerId(uint32_t player_id) {
    player_id_ = player_id;
    player_id_set_ = true;
  }

  virtual void PreSolve(b2Contact* contact, const b2Manifold* old_manifold) {
    if (!player_id_set_) {
      contact->SetEnabled(false);
      return;
    }

    Entity* a = static_cast<Entity*>(
        contact->GetFixtureA()->GetBody()->GetUserData());
    Entity* b = static_cast<Entity*>(
        contact->GetFixtureB()->GetBody()->GetUserData());

    if (player_id_set_ && a->GetId() != player_id_ &&
                          b->GetId() != player_id_) {
      // Nothing except our player collides.
      contact->SetEnabled(false);
      return;
    }

    if (a->GetType() == Entity::TYPE_PLAYER &&
        (b->GetType() == Entity::TYPE_WALL ||
         b->GetType() == Entity::TYPE_ACTIVATOR)) {
      // Our player collides only with static entities.
      return;
    }

    if (b->GetType() == Entity::TYPE_PLAYER &&
        (a->GetType() == Entity::TYPE_WALL ||
         a->GetType() == Entity::TYPE_ACTIVATOR)) {
      // Our player collides only with static entities.
      return;
    }

    contact->SetEnabled(false);
  }

  virtual void BeginContact(b2Contact* contact) { }
  virtual void EndContact(b2Contact* contact) { }

 private:
  bool player_id_set_;
  uint32_t player_id_;
};

}  // namespace bm

#endif  // CLIENT_CONTACT_LISTENER_H_
