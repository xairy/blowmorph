// Copyright (c) 2015 Blowmorph Team

#include "server/controller.h"

#include <cmath>
#include <cstdlib>

#include <map>
#include <string>
#include <vector>

#include <pugixml.hpp>

#include <Box2D/Box2D.h>

#include "base/error.h"
#include "base/macros.h"
#include "base/pstdint.h"
#include "base/settings_manager.h"
#include "base/utils.h"

#include "server/entity.h"
#include "server/id_manager.h"

#include "server/activator.h"
#include "server/bullet.h"
#include "server/critter.h"
#include "server/kit.h"
#include "server/player.h"
#include "server/wall.h"

namespace bm {

Controller::Controller(IdManager* id_manager) : world_(this, id_manager) {
  world_.GetBox2DWorld()->SetContactListener(&contact_listener_);
  bool rv = settings_.Open("data/entities.cfg");
  CHECK(rv == true);  // FIXME(xairy).
}

Controller::~Controller() { }

World* Controller::GetWorld() {
  return &world_;
}

SettingsManager* Controller::GetSettings() {
  return &settings_;
}

std::vector<GameEvent>* Controller::GetGameEvents() {
  return &game_events_;
}

void Controller::Update(int64_t time, int64_t time_delta) {
  SpawnZombies();
  UpdateEntities(time_delta);
  StepPhysics(time_delta);
  DestroyOutlyingEntities();
  RespawnDeadPlayers();
  DeleteDestroyedEntities(time, time_delta);

  // TODO(xairy): refactor.
  std::vector<b2Vec2>::iterator it;
  for (it = morph_list_.begin(); it != morph_list_.end(); ++it) {
    MakeSlimeExplosion(*it);
  }
  morph_list_.clear();
}

Player* Controller::OnPlayerConnected() {
  Player* player = world_.CreatePlayer(b2Vec2(0.0f, 0.0f));
  RespawnPlayer(player);
  OnEntityAppearance(player);
  return player;
}

void Controller::OnPlayerDisconnected(Player* player) {
  player->Destroy();
}

void Controller::OnEntityAppearance(Entity* entity) {
  std::map<uint32_t, Entity*>::iterator itr, end;
  end = world_.GetDynamicEntities()->end();
  for (itr = world_.GetDynamicEntities()->begin(); itr != end; ++itr) {
    Entity::Type itr_type = itr->second->GetType();
    Entity::Type ent_type = entity->GetType();
    if ((itr_type == Entity::TYPE_CRITTER && ent_type == Entity::TYPE_PLAYER) ||
        (itr_type == Entity::TYPE_PLAYER && ent_type == Entity::TYPE_CRITTER)) {
      Critter* critter = NULL;
      Player* player = NULL;
      if (itr_type == Entity::TYPE_CRITTER) {
        critter = static_cast<Critter*>(itr->second);
        player = static_cast<Player*>(entity);
      } else {
        critter = static_cast<Critter*>(entity);
        player = static_cast<Player*>(itr->second);
      }
      if (critter->GetTarget() == NULL) {
         critter->SetTarget(player);
      } else {
        float current_distance = (critter->GetTarget()->GetPosition() -
            critter->GetPosition()).Length();
        float new_distance = (player->GetPosition() -
            critter->GetPosition()).Length();
        if (new_distance < current_distance) {
          critter->SetTarget(player);
        }
      }
    }
  }
}

void Controller::OnEntityDisappearance(Entity* entity) {
  std::map<uint32_t, Entity*>::iterator itr, end;
  end = world_.GetDynamicEntities()->end();
  for (itr = world_.GetDynamicEntities()->begin(); itr != end; ++itr) {
    Entity::Type itr_type = itr->second->GetType();
    Entity::Type ent_type = entity->GetType();
    if ((itr_type == Entity::TYPE_CRITTER && ent_type == Entity::TYPE_PLAYER) ||
        (itr_type == Entity::TYPE_PLAYER && ent_type == Entity::TYPE_CRITTER)) {
      Critter* critter = NULL;
      Player* player = NULL;
      if (itr_type == Entity::TYPE_CRITTER) {
        critter = static_cast<Critter*>(itr->second);
        player = static_cast<Player*>(entity);
      } else {
        critter = static_cast<Critter*>(entity);
        player = static_cast<Player*>(itr->second);
      }
      if (critter->GetTarget() == player) {
         critter->SetTarget(NULL);
      }
    }
  }
}

void Controller::OnKeyboardEvent(Player* player, const KeyboardEvent& event) {
  player->OnKeyboardEvent(event);
}

void Controller::OnMouseEvent(Player* player, const MouseEvent& event) {
  int bazooka_consumption =
      settings_.GetInt32("player.bazooka.energy_consumption");
  int morpher_consumption =
      settings_.GetInt32("player.morpher.energy_consumption");

  if (event.event_type == MouseEvent::EVENT_KEYDOWN &&
    event.button_type == MouseEvent::BUTTON_LEFT) {
    if (player->GetEnergy() >= bazooka_consumption) {
      player->AddEnergy(-bazooka_consumption);
      b2Vec2 start = player->GetPosition();
      b2Vec2 end(static_cast<float>(event.x), static_cast<float>(event.y));
      Bullet* bullet = world_.CreateBullet(player->GetId(),
          start, end, Bullet::TYPE_ROCKET);
      OnEntityAppearance(bullet);
    }
  }
  if (event.event_type == MouseEvent::EVENT_KEYDOWN &&
    event.button_type == MouseEvent::BUTTON_RIGHT) {
    if (player->GetEnergy() >= morpher_consumption) {
      player->AddEnergy(-morpher_consumption);
      b2Vec2 start = player->GetPosition();
      b2Vec2 end(static_cast<float>(event.x), static_cast<float>(event.y));
      Bullet* bullet = world_.CreateBullet(player->GetId(),
          start, end, Bullet::TYPE_SLIME);
      OnEntityAppearance(bullet);
    }
  }
  if (event.event_type == MouseEvent::EVENT_MOVE) {
    b2Vec2 mouse_position = b2Vec2(event.x, event.y);
    b2Vec2 direction = mouse_position - player->GetPosition();
    float angle = atan2f(-direction.x, direction.y);
    player->SetRotation(angle / M_PI * 180);
  }
}

void Controller::OnPlayerAction(Player* player, const PlayerAction& event) {
  if (event.type == PlayerAction::TYPE_ACTIVATE) {
    Entity* entity = world_.GetEntity(event.target_id);
    if (entity == NULL) {
      return;
    }
    if (entity->GetType() != Entity::TYPE_ACTIVATOR) {
      return;
    }
    Activator* activator = static_cast<Activator*>(entity);
    activator->Activate(player);
  }
}

// Collisions.

void Controller::OnCollision(Activator* activator1, Activator* activator2) { }
void Controller::OnCollision(Activator* activator, Kit* kit) { }
void Controller::OnCollision(Activator* activator, Wall* wall) { }
void Controller::OnCollision(Activator* activator, Player* player) { }
void Controller::OnCollision(Activator* activator, Critter* critter) { }
void Controller::OnCollision(Activator* activator, Bullet* bullet) { }

void Controller::OnCollision(Kit* kit1, Kit* kit2) { }
void Controller::OnCollision(Kit* kit, Wall* wall) { }

void Controller::OnCollision(Kit* kit, Player* player) {
  player->AddHealth(kit->GetHealthRegeneration());
  player->AddEnergy(kit->GetEnergyRegeneration());
  kit->Destroy();
}

void Controller::OnCollision(Kit* kit, Critter* critter) { }
void Controller::OnCollision(Kit* kit, Bullet* bullet) { }

void Controller::OnCollision(Wall* wall1, Wall* wall2) { }
void Controller::OnCollision(Wall* wall, Player* player) { }

void Controller::OnCollision(Wall* wall, Critter* critter) {
  ExplodeCritter(critter);
}

void Controller::OnCollision(Wall* wall, Bullet* bullet) {
  ExplodeBullet(bullet);
}

void Controller::OnCollision(Player* player1, Player* player2) { }

void Controller::OnCollision(Player* player, Critter* critter) {
  ExplodeCritter(critter);
}

void Controller::OnCollision(Player* player, Bullet* bullet) {
  if (bullet->GetOwnerId() == player->GetId()) {
    return;
  }
  ExplodeBullet(bullet);
}

void Controller::OnCollision(Critter* critter1, Critter* critter2) { }

void Controller::OnCollision(Critter* critter, Bullet* bullet) {
  ExplodeBullet(bullet);
  ExplodeCritter(critter);
}

void Controller::OnCollision(Bullet* bullet1, Bullet* bullet2) {
  ExplodeBullet(bullet1);
  ExplodeBullet(bullet2);
}

// Updating.

void Controller::SpawnZombies() {
  static int counter = 0;
  if (counter == 300) {
    float x = -250.0f + static_cast<float>(rand()) / RAND_MAX * 500.0f;  // NOLINT
    float y = -250.0f + static_cast<float>(rand()) / RAND_MAX * 500.0f;  // NOLINT
    Critter* critter = world_.CreateCritter(b2Vec2(x, y));
    OnEntityAppearance(critter);
    counter = 0;
  }
  counter++;
}

void Controller::UpdateEntities(int64_t time_delta) {
  std::map<uint32_t, Entity*>::iterator i, end;
  end = world_.GetDynamicEntities()->end();
  for (i = world_.GetDynamicEntities()->begin(); i != end; ++i) {
    Entity* entity = i->second;
    if (entity->GetType() == Entity::TYPE_CRITTER) {
      Critter* critter = static_cast<Critter*>(entity);
      Entity* target = critter->GetTarget();
      if (target != NULL) {
        b2Vec2 velocity = target->GetPosition() - critter->GetPosition();
        velocity.Normalize();
        velocity *= critter->GetSpeed();
        critter->SetImpulse(critter->GetMass() * velocity);
        float angle = atan2f(-velocity.x, velocity.y);
        critter->SetRotation(angle / M_PI * 180);
      }
    } else if (entity->GetType() == Entity::TYPE_PLAYER) {
      Player* player = static_cast<Player*>(entity);
      Player::KeyboardState* keyboard_state = player->GetKeyboardState();
      float speed = player->GetSpeed();
      b2Vec2 velocity;
      velocity.x = keyboard_state->left * (-speed)
        + keyboard_state->right * (speed);
      velocity.y = keyboard_state->up * (-speed)
        + keyboard_state->down * (speed);
      player->SetImpulse(player->GetMass() * velocity);
      player->Regenerate(time_delta);
    }
  }
}

void Controller::StepPhysics(int64_t time_delta) {
  int32_t velocity_iterations = 6;
  int32_t position_iterations = 2;
  world_.GetBox2DWorld()->Step(static_cast<float>(time_delta) / 1000,
      velocity_iterations, position_iterations);
}

void Controller::DestroyOutlyingEntities() {
  float bound = world_.GetBound();
  std::map<uint32_t, Entity*>::iterator i, end;
  end = world_.GetStaticEntities()->end();
  for (i = world_.GetStaticEntities()->begin(); i != end; ++i) {
    Entity* entity = i->second;
    b2Vec2 position = entity->GetPosition();
    if (abs(position.x) > bound || abs(position.y) > bound) {
      entity->Destroy();
    }
  }
  end = world_.GetDynamicEntities()->end();
  for (i = world_.GetDynamicEntities()->begin(); i != end; ++i) {
    Entity* entity = i->second;
    b2Vec2 position = entity->GetPosition();
    if (abs(position.x) > bound || abs(position.y) > bound) {
      if (entity->GetType() != Entity::TYPE_PLAYER) {
        entity->Destroy();
      }
    }
  }
}

void Controller::RespawnDeadPlayers() {
  std::map<uint32_t, Entity*>::iterator i, end;
  end = world_.GetDynamicEntities()->end();
  for (i = world_.GetDynamicEntities()->begin(); i != end; ++i) {
    Entity* entity = i->second;
    if (entity->GetType() == Entity::TYPE_PLAYER) {
      Player* player = static_cast<Player*>(entity);
      if (player->GetHealth() <= 0) {
        RespawnPlayer(player);
        UpdateScore(player);
      }
    }
  }
}

void Controller::RespawnPlayer(Player* player) {
  CHECK(world_.GetSpawnPositions()->size() > 0);
  size_t spawn_count = world_.GetSpawnPositions()->size();
  size_t spawn = Random(spawn_count);
  player->SetPosition(world_.GetSpawnPositions()->at(spawn));
  player->RestoreHealth();
}

void Controller::UpdateScore(Player* player) {
  uint32_t killer_id = player->GetKillerId();
  if (killer_id == player->GetId()) {
    player->DecScore();
  } else {
    Entity* entity = world_.GetEntity(killer_id);
    if (entity != NULL && entity->GetType() == Entity::TYPE_PLAYER) {
      Player* killer = static_cast<Player*>(entity);
      killer->IncScore();
    }
  }
}

void Controller::DeleteDestroyedEntities(int64_t time, int64_t time_delta) {
  std::map<uint32_t, Entity*>::iterator itr, end;
  end = world_.GetStaticEntities()->end();
  for (itr = world_.GetStaticEntities()->begin(); itr != end; ++itr) {
    Entity* entity = itr->second;
    if (entity->IsDestroyed()) {
      GameEvent event;
      event.type = GameEvent::TYPE_ENTITY_DISAPPEARED;
      event.x = entity->GetPosition().x;
      event.y = entity->GetPosition().y;
      entity->GetSnapshot(time + time_delta, &event.entity);
      game_events_.push_back(event);
      world_.RemoveEntity(entity->GetId());
      OnEntityDisappearance(entity);
      delete entity;
    }
  }
  end = world_.GetDynamicEntities()->end();
  for (itr = world_.GetDynamicEntities()->begin(); itr != end; ++itr) {
    Entity* entity = itr->second;
    if (entity->IsDestroyed()) {
      GameEvent event;
      event.type = GameEvent::TYPE_ENTITY_DISAPPEARED;
      event.x = entity->GetPosition().x;
      event.y = entity->GetPosition().y;
      entity->GetSnapshot(time + time_delta, &event.entity);
      game_events_.push_back(event);
      world_.RemoveEntity(entity->GetId());
      OnEntityDisappearance(entity);
      delete entity;
    }
  }
}

// Explosions.

void Controller::ExplodeBullet(Bullet* bullet) {
  // We do not want 'bullet' to explode multiple times.
  if (!bullet->IsDestroyed()) {
    if (bullet->GetBulletType() == Bullet::TYPE_ROCKET) {
      MakeExplosion(bullet->GetPosition(), bullet->GetOwnerId());
    } else if (bullet->GetBulletType() == Bullet::TYPE_SLIME) {
      morph_list_.push_back(bullet->GetPosition());
    }
    bullet->Destroy();
  }
}

void Controller::ExplodeCritter(Critter* critter) {
  // We do not want 'critter' to explode multiple times.
  if (!critter->IsDestroyed()) {
    MakeExplosion(critter->GetPosition(), critter->GetId());
    critter->Destroy();
  }
}

void Controller::MakeExplosion(const b2Vec2& location, uint32_t source_id) {
  float radius = settings_.GetFloat("player.bazooka.explosion_radius");
  int damage = settings_.GetInt32("player.bazooka.explosion_damage");

  // FIXME(xairy): can miss huge entities.
  radius += 13.0f;

  std::map<uint32_t, Entity*>::iterator i, end;
  end = world_.GetStaticEntities()->end();
  for (i = world_.GetStaticEntities()->begin(); i != end; ++i) {
    Entity* entity = i->second;
    float distance2 = (entity->GetPosition() - location).LengthSquared();
    if (distance2 <= radius * radius) {
      entity->Damage(damage, source_id);
    }
  }
  end = world_.GetDynamicEntities()->end();
  for (i = world_.GetDynamicEntities()->begin(); i != end; ++i) {
    Entity* entity = i->second;
    float distance2 = (entity->GetPosition() - location).LengthSquared();
    if (distance2 <= radius * radius) {
      entity->Damage(damage, source_id);
    }
  }

  GameEvent event;
  event.x = location.x;
  event.y = location.y;
  event.type = GameEvent::TYPE_EXPLOSION;
  game_events_.push_back(event);
}

void Controller::MakeSlimeExplosion(const b2Vec2& location) {
  int radius = settings_.GetInt32("player.morpher.radius");
  float block_size = world_.GetBlockSize();
  int lx = static_cast<int>(round(location.x / block_size));
  int ly = static_cast<int>(round(location.y / block_size));
  for (int x = -radius; x <= radius; x++) {
    for (int y = -radius; y <= radius; y++) {
      if (x * x + y * y <= radius * radius) {
        Wall* wall = world_.CreateWall(b2Vec2((lx + x) * block_size,
          (ly + y) * block_size), Wall::TYPE_MORPHED);
        OnEntityAppearance(wall);
      }
    }
  }
}

}  // namespace bm
