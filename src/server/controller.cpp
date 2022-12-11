// Copyright (c) 2015 Blowmorph Team

#include "server/controller.h"

#include <cmath>
#include <cstdlib>

#include <map>
#include <string>
#include <vector>
#include <utility>

#include <Box2D/Box2D.h>

#include "base/error.h"
#include "base/macros.h"
#include "base/pstdint.h"
#include "base/utils.h"

#include "engine/config.h"
#include "engine/utils.h"

#include "server/entity.h"

#include "server/activator.h"
#include "server/critter.h"
#include "server/door.h"
#include "server/kit.h"
#include "server/projectile.h"
#include "server/player.h"
#include "server/wall.h"

namespace bm {

Controller::Controller() : world_(this) {
  world_.GetBox2DWorld()->SetContactListener(&contact_listener_);
}

Controller::~Controller() { }

ServerWorld* Controller::GetWorld() {
  return &world_;
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
  std::vector<std::pair<b2Vec2, int> >::iterator it;
  for (it = morph_list_.begin(); it != morph_list_.end(); ++it) {
    MakeSlimeExplosion(it->first, it->second);
  }
  morph_list_.clear();
}

Player* Controller::OnPlayerConnected() {
  Player* player = world_.CreatePlayer(b2Vec2(0.0f, 0.0f), "player");
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
  if (event.event_type == MouseEvent::EVENT_KEYDOWN) {
    std::string gun_name;
    if (event.button_type == MouseEvent::BUTTON_LEFT) {
      gun_name = "bazooka";
    } else if (event.button_type == MouseEvent::BUTTON_RIGHT) {
      gun_name = "morpher";
    }

    auto config = Config::GetInstance()->GetGunsConfig().at(gun_name);
    int energy_consumption = config.energy_consumption;
    std::string projectile_config = config.projectile_name;

    if (player->GetEnergy() >= energy_consumption) {
      player->AddEnergy(-energy_consumption);
      b2Vec2 start = player->GetPosition();
      b2Vec2 end(static_cast<float>(event.x), static_cast<float>(event.y));
      Projectile* projectile = world_.CreateProjectile(player->GetId(),
          start, end, projectile_config);
      OnEntityAppearance(projectile);
    }
  }

  if (event.event_type == MouseEvent::EVENT_MOVE) {
    b2Vec2 mouse_position = b2Vec2(event.x, event.y);
    b2Vec2 direction = mouse_position - player->GetPosition();
    float angle = atan2f(-direction.x, direction.y);
    player->SetRotation(angle);
  }
}

void Controller::OnPlayerAction(Player* player, const PlayerAction& event) {
  if (event.type == PlayerAction::TYPE_ACTIVATE) {
    Entity* entity = world_.GetEntity(event.target_id);
    if (entity == NULL) {
      return;
    }

    // Check visibility.
    b2Body* body = RayCast(world_.GetBox2DWorld(), player->GetPosition(),
      entity->GetPosition());
    if (body == NULL) {
      return;
    }
    if (entity != static_cast<Entity*>(body->GetUserData())) {
      return;
    }

    if (entity->GetType() == Entity::TYPE_ACTIVATOR) {
      Activator* activator = static_cast<Activator*>(entity);

      // Check distance.
      if (Length(entity->GetPosition() - player->GetPosition()) >
          activator->GetActivationDistance()) {
        return;
      }

      activator->Activate(player);
    } else if (entity->GetType() == Entity::TYPE_DOOR) {
      Door* door = static_cast<Door*>(entity);

      // Check distance.
      if (Length(entity->GetPosition() - player->GetPosition()) >
          door->GetActivationDistance()) {
        return;
      }

      door->Activate(player);
    }
  }
}

// Collisions.

void Controller::OnCollision(Door* first, Door* second) { }
void Controller::OnCollision(Door* first, Activator* second) { }
void Controller::OnCollision(Door* first, Kit* second) { }
void Controller::OnCollision(Door* first, Wall* second) { }
void Controller::OnCollision(Door* first, Player* second) { }
void Controller::OnCollision(Door* first, Critter* second) { }

void Controller::OnCollision(Door* first, Projectile* second) {
  DestroyProjectile(second);
}

void Controller::OnCollision(Activator* first, Activator* second) { }
void Controller::OnCollision(Activator* first, Kit* second) { }
void Controller::OnCollision(Activator* first, Wall* second) { }
void Controller::OnCollision(Activator* first, Player* second) { }
void Controller::OnCollision(Activator* first, Critter* second) { }

void Controller::OnCollision(Activator* first, Projectile* second) {
  DestroyProjectile(second);
}

void Controller::OnCollision(Kit* first, Kit* second) { }
void Controller::OnCollision(Kit* first, Wall* second) { }

void Controller::OnCollision(Kit* first, Player* second) {
  second->AddHealth(first->GetHealthRegeneration());
  second->AddEnergy(first->GetEnergyRegeneration());
  first->Destroy();
}

void Controller::OnCollision(Kit* first, Critter* second) { }
void Controller::OnCollision(Kit* first, Projectile* second) { }

void Controller::OnCollision(Wall* first, Wall* second) { }
void Controller::OnCollision(Wall* first, Player* second) { }

void Controller::OnCollision(Wall* first, Critter* second) {
  second->Destroy();
}

void Controller::OnCollision(Wall* first, Projectile* second) {
  DestroyProjectile(second);
}

void Controller::OnCollision(Player* first, Player* second) { }

void Controller::OnCollision(Player* first, Critter* second) {
  // FIXME(xairy): load damage from config.
  first->Damage(30, second->GetId());
  second->Destroy();
}

void Controller::OnCollision(Player* first, Projectile* second) {
  if (second->GetOwnerId() == first->GetId()) {
    return;
  }
  DestroyProjectile(second);
}

void Controller::OnCollision(Critter* first, Critter* second) { }

void Controller::OnCollision(Critter* first, Projectile* second) {
  DestroyProjectile(second);
  first->Destroy();
}

void Controller::OnCollision(Projectile* first, Projectile* second) {
  DestroyProjectile(first);
  DestroyProjectile(second);
}

// Updating.

void Controller::SpawnZombies() {

  static int counter = 0;
  if (counter == 300) {
    bool player_found = false;
    for (auto i : *world_.GetDynamicEntities()) {
      ServerEntity* entity = static_cast<ServerEntity*>(i.second);
      if (entity->GetType() == Entity::TYPE_PLAYER) {
        player_found = true;
	break;
      }
    }
    if (!player_found) {
      return;
    }
    for (auto spawn : *world_.GetZombieSpawnPositions()) {
      Critter* critter = world_.CreateCritter(spawn, "zombie");
      OnEntityAppearance(critter);
    }
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
        critter->SetRotation(angle);
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
  for (auto i : *world_.GetStaticEntities()) {
    ServerEntity* entity = static_cast<ServerEntity*>(i.second);
    b2Vec2 position = entity->GetPosition();
    if (std::abs(position.x) > bound || std::abs(position.y) > bound) {
      entity->Destroy();
    }
  }
  for (auto i : *world_.GetDynamicEntities()) {
    ServerEntity* entity = static_cast<ServerEntity*>(i.second);
    b2Vec2 position = entity->GetPosition();
    if (std::abs(position.x) > bound || std::abs(position.y) > bound) {
      if (entity->GetType() != Entity::TYPE_PLAYER) {
        entity->Destroy();
      }
    }
  }
}

void Controller::RespawnDeadPlayers() {
  for (auto i : *world_.GetDynamicEntities()) {
    ServerEntity* entity = static_cast<ServerEntity*>(i.second);
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
  for (itr = world_.GetStaticEntities()->begin(); itr != end;) {
    ServerEntity* entity = static_cast<ServerEntity*>(itr->second);
    ++itr;
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
  for (itr = world_.GetDynamicEntities()->begin(); itr != end;) {
    ServerEntity* entity = static_cast<ServerEntity*>(itr->second);
    ++itr;
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

void Controller::DestroyProjectile(Projectile* projectile) {
  // We do not want 'projectile' to explode multiple times.
  if (!projectile->IsDestroyed()) {
    if (projectile->GetProjectileType() == Projectile::TYPE_ROCKET) {
      MakeRocketExplosion(projectile->GetPosition(),
        projectile->GetRocketExplosionRadius(),
        projectile->GetRocketExplosionDamage(),
        projectile->GetOwnerId());
    } else if (projectile->GetProjectileType() == Projectile::TYPE_SLIME) {
      morph_list_.push_back(
        std::pair<b2Vec2, int>(projectile->GetPosition(),
          projectile->GetSlimeExplosionRadius()));
    }
    projectile->Destroy();
  }
}

void Controller::MakeRocketExplosion(const b2Vec2& location, float radius,
                               int damage, uint32_t source_id) {
  // FIXME(xairy): can miss huge entities.
  radius += 13.0f;

  for (auto i : *world_.GetStaticEntities()) {
    ServerEntity* entity = static_cast<ServerEntity*>(i.second);
    float distance2 = (entity->GetPosition() - location).LengthSquared();
    if (distance2 <= radius * radius) {
      entity->Damage(damage, source_id);
    }
  }
  for (auto i : *world_.GetDynamicEntities()) {
    ServerEntity* entity = static_cast<ServerEntity*>(i.second);
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

void Controller::MakeSlimeExplosion(const b2Vec2& location, int radius) {
  float block_size = 16.0f;
  int lx = static_cast<int>(round(location.x / block_size));
  int ly = static_cast<int>(round(location.y / block_size));
  for (int x = -radius; x <= radius; x++) {
    for (int y = -radius; y <= radius; y++) {
      if (x * x + y * y <= radius * radius) {
        Wall* wall = world_.CreateWall(b2Vec2((lx + x) * block_size,
          (ly + y) * block_size), "morphed_wall");
        OnEntityAppearance(wall);
      }
    }
  }
}

}  // namespace bm
