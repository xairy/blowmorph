// Copyright (c) 2015 Blowmorph Team

#ifndef CLIENT_CONTACT_LISTENER_H_
#define CLIENT_CONTACT_LISTENER_H_

#include <Box2D/Box2D.h>

#include "base/pstdint.h"

#include "client/object.h"

namespace bm {

class ContactListener : public b2ContactListener {
 public:
  ContactListener() : player_id_set_(false), player_id_(0) { }

  void SetPlayerId(uint32_t player_id) {
    player_id_ = player_id;
    player_id_set_ = true;
  }

  virtual void PreSolve(b2Contact* contact, const b2Manifold* old_manifold) {
    Object* a = static_cast<Object*>(
        contact->GetFixtureA()->GetBody()->GetUserData());
    Object* b = static_cast<Object*>(
        contact->GetFixtureB()->GetBody()->GetUserData());
    if (player_id_set_ && a->id != player_id_ && b->id != player_id_) {
      // Nothing except our player collides.
      contact->SetEnabled(false);
      return;
    }
    if (a->GetType() != Object::TYPE_PLAYER ||
        b->GetType() != Object::TYPE_WALL) {
      // Our player collides only with walls.
      contact->SetEnabled(false);
    }
  }

  virtual void BeginContact(b2Contact* contact) { }
  virtual void EndContact(b2Contact* contact) { }

 private:
  bool player_id_set_;
  uint32_t player_id_;
};

}  // namespace bm

#endif  // CLIENT_CONTACT_LISTENER_H_
