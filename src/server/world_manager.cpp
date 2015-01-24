// Copyright (c) 2015 Blowmorph Team

#include "server/world_manager.h"

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

#include "server/entity.h"
#include "server/id_manager.h"

#include "server/activator.h"
#include "server/bullet.h"
#include "server/dummy.h"
#include "server/kit.h"
#include "server/player.h"
#include "server/wall.h"

namespace {

// TODO(xairy): move to utils.

// XXX(xairy): shouldn't it come from math.h?
double round(double value) {
  return ::floor(value + 0.5);
}
float round(float value) {
  return ::floorf(value + 0.5f);
}

// Returns random number in the range [0, max).
// XXX(xairy): not thread safe because of rand().
size_t Random(size_t max) {
  CHECK(max > 0);
  double zero_to_one = static_cast<double>(rand()) /  // NOLINT
    (static_cast<double>(RAND_MAX) + 1.0f);
  return static_cast<size_t>(zero_to_one * max);
}

}  // anonymous namespace

namespace bm {

WorldManager::WorldManager(IdManager* id_manager)
    : world_(b2Vec2(0.0f, 0.0f)), _map_type(MAP_NONE), _id_manager(id_manager) {
  world_.SetContactListener(&contact_listener_);
  bool rv = _settings.Open("data/entities.cfg");
  CHECK(rv == true);  // FIXME(xairy).

  // FIXME(xairy): Temporary.
  CreateActivator(b2Vec2(0, 0), Activator::TYPE_DOOR);
}

WorldManager::~WorldManager() {
  std::map<uint32_t, Entity*>::iterator i, end;
  end = _static_entities.end();
  for (i = _static_entities.begin(); i != end; ++i) {
    delete i->second;
  }
  end = _dynamic_entities.end();
  for (i = _dynamic_entities.begin(); i != end; ++i) {
    delete i->second;
  }
}

b2World* WorldManager::GetWorld() {
  return &world_;
}

SettingsManager* WorldManager::GetSettings() {
  return &_settings;
}

void WorldManager::AddEntity(uint32_t id, Entity* entity) {
  CHECK(_static_entities.count(id) == 0 && _dynamic_entities.count(id) == 0);
  if (entity->IsStatic()) {
    _static_entities[id] = entity;
  } else {
    _dynamic_entities[id] = entity;
  }
  OnEntityAppearance(entity);
}

void WorldManager::DeleteEntity(uint32_t id, bool deallocate) {
  CHECK(_static_entities.count(id) + _dynamic_entities.count(id) == 1);
  Entity* entity = NULL;
  if (_static_entities.count(id) == 1) {
    entity = _static_entities[id];
    _static_entities.erase(id);
  } else if (_dynamic_entities.count(id) == 1) {
    entity = _dynamic_entities[id];
    _dynamic_entities.erase(id);
  }
  CHECK(entity != NULL);

  OnEntityDisappearance(entity);

  if (deallocate) {
    delete entity;
  }
}

void WorldManager::OnEntityAppearance(Entity* entity) {
  std::map<uint32_t, Entity*>::iterator itr, end;
  end = _dynamic_entities.end();
  for (itr = _dynamic_entities.begin(); itr != end; ++itr) {
    Entity::Type itr_type = itr->second->GetType();
    Entity::Type ent_type = entity->GetType();
    if ((itr_type == Entity::TYPE_DUMMY && ent_type == Entity::TYPE_PLAYER) ||
        (itr_type == Entity::TYPE_PLAYER && ent_type == Entity::TYPE_DUMMY)) {
      Dummy* dummy = NULL;
      Player* player = NULL;
      if (itr_type == Entity::TYPE_DUMMY) {
        dummy = static_cast<Dummy*>(itr->second);
        player = static_cast<Player*>(entity);
      } else {
        dummy = static_cast<Dummy*>(entity);
        player = static_cast<Player*>(itr->second);
      }
      if (dummy->GetTarget() == NULL) {
         dummy->SetTarget(player);
      } else {
        float current_distance = (dummy->GetTarget()->GetPosition() -
            dummy->GetPosition()).Length();
        float new_distance = (player->GetPosition() -
            dummy->GetPosition()).Length();
        if (new_distance < current_distance) {
          dummy->SetTarget(player);
        }
      }
    }
  }
}

void WorldManager::OnEntityDisappearance(Entity* entity) {
  std::map<uint32_t, Entity*>::iterator itr, end;
  end = _dynamic_entities.end();
  for (itr = _dynamic_entities.begin(); itr != end; ++itr) {
    Entity::Type itr_type = itr->second->GetType();
    Entity::Type ent_type = entity->GetType();
    if ((itr_type == Entity::TYPE_DUMMY && ent_type == Entity::TYPE_PLAYER) ||
        (itr_type == Entity::TYPE_PLAYER && ent_type == Entity::TYPE_DUMMY)) {
      Dummy* dummy = NULL;
      Player* player = NULL;
      if (itr_type == Entity::TYPE_DUMMY) {
        dummy = static_cast<Dummy*>(itr->second);
        player = static_cast<Player*>(entity);
      } else {
        dummy = static_cast<Dummy*>(entity);
        player = static_cast<Player*>(itr->second);
      }
      if (dummy->GetTarget() == player) {
         dummy->SetTarget(NULL);
      }
    }
  }
}

void WorldManager::DeleteEntities(const std::vector<uint32_t>& input,
    bool deallocate) {
  size_t size = input.size();
  for (size_t i = 0; i < size; i++) {
    DeleteEntity(input[i], deallocate);
  }
}

Entity* WorldManager::GetEntity(uint32_t id) {
  if (_static_entities.count(id) == 1) {
    return _static_entities[id];
  } else if (_dynamic_entities.count(id) == 1) {
    return _dynamic_entities[id];
  }
  return NULL;
}

std::map<uint32_t, Entity*>* WorldManager::GetStaticEntities() {
  return &_static_entities;
}

std::map<uint32_t, Entity*>* WorldManager::GetDynamicEntities() {
  return &_dynamic_entities;
}

void WorldManager::GetDestroyedEntities(std::vector<uint32_t>* output) {
  output->clear();
  output->reserve(128);

  std::map<uint32_t, Entity*>::iterator itr, end;
  end = _static_entities.end();
  for (itr = _static_entities.begin(); itr != end; ++itr) {
    if (itr->second->IsDestroyed()) {
      output->push_back(itr->first);
    }
  }
  end = _dynamic_entities.end();
  for (itr = _dynamic_entities.begin(); itr != end; ++itr) {
    if (itr->second->IsDestroyed()) {
      output->push_back(itr->first);
    }
  }
}

std::vector<GameEvent>* WorldManager::GetGameEvents() {
  return &_game_events;
}

void WorldManager::Update(int64_t time_delta) {
  // XXX(xairy): Temporary.
  static int counter = 0;
  if (counter == 300) {
    float x = -250.0f + static_cast<float>(rand()) / RAND_MAX * 500.0f;  // NOLINT
    float y = -250.0f + static_cast<float>(rand()) / RAND_MAX * 500.0f;  // NOLINT
    CreateDummy(b2Vec2(x, y));
    counter = 0;
  }
  counter++;

  UpdateEntities(time_delta);
  StepPhysics(time_delta);
  DestroyOutlyingEntities();
  RespawnDeadPlayers();

  // TODO(xairy): refactor.
  std::vector<b2Vec2>::iterator it;
  for (it = _morph_list.begin(); it != _morph_list.end(); ++it) {
    MakeSlimeExplosion(*it);
  }
  _morph_list.clear();
}

void WorldManager::UpdateEntities(int64_t time_delta) {
  std::map<uint32_t, Entity*>::iterator i, end;
  end = _dynamic_entities.end();
  for (i = _dynamic_entities.begin(); i != end; ++i) {
    Entity* entity = i->second;
    if (entity->GetType() == Entity::TYPE_DUMMY) {
      Dummy* dummy = static_cast<Dummy*>(entity);
      Entity* target = dummy->GetTarget();
      if (target != NULL) {
        b2Vec2 velocity = target->GetPosition() - dummy->GetPosition();
        velocity.Normalize();
        velocity *= dummy->GetSpeed();
        dummy->SetImpulse(dummy->GetMass() * velocity);
        float angle = atan2f(-velocity.x, velocity.y);
        dummy->SetRotation(angle / M_PI * 180);
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

void WorldManager::StepPhysics(int64_t time_delta) {
  int32_t velocity_iterations = 6;
  int32_t position_iterations = 2;
  world_.Step(static_cast<float>(time_delta) / 1000,
      velocity_iterations, position_iterations);
}

void WorldManager::DestroyOutlyingEntities() {
  std::map<uint32_t, Entity*>::iterator i, end;
  end = _static_entities.end();
  for (i = _static_entities.begin(); i != end; ++i) {
    Entity* entity = i->second;
    b2Vec2 position = entity->GetPosition();
    if (abs(position.x) > _bound || abs(position.y) > _bound) {
      entity->Destroy();
    }
  }
  end = _dynamic_entities.end();
  for (i = _dynamic_entities.begin(); i != end; ++i) {
    Entity* entity = i->second;
    b2Vec2 position = entity->GetPosition();
    if (abs(position.x) > _bound || abs(position.y) > _bound) {
      if (entity->GetType() != Entity::TYPE_PLAYER) {
        entity->Destroy();
      }
    }
  }
}

void WorldManager::CreateBullet(
  uint32_t owner_id,
  const b2Vec2& start,
  const b2Vec2& end,
  Bullet::Type type
) {
  CHECK(_static_entities.count(owner_id) +
    _dynamic_entities.count(owner_id) == 1);
  uint32_t id = _id_manager->NewId();
  Bullet* bullet = new Bullet(this, id, owner_id, start, end, type);
  CHECK(bullet != NULL);
  AddEntity(id, bullet);
}

void WorldManager::CreateDummy(
  const b2Vec2& position
) {
  uint32_t id = _id_manager->NewId();
  Dummy* dummy = new Dummy(this, id, position);
  CHECK(dummy != NULL);
  AddEntity(id, dummy);
}

void WorldManager::CreateWall(
  const b2Vec2& position,
  Wall::Type type
) {
  uint32_t id = _id_manager->NewId();
  Wall* wall = new Wall(this, id, position, type);
  CHECK(wall != NULL);
  AddEntity(id, wall);
}

void WorldManager::CreateKit(
  const b2Vec2& position,
  int health_regeneration,
  int energy_regeneration,
  Kit::Type type
) {
  uint32_t id = _id_manager->NewId();
  Kit* kit = new Kit(this, id, position, health_regeneration,
    energy_regeneration, type);
  CHECK(kit != NULL);
  AddEntity(id, kit);
}

void WorldManager::CreateActivator(
  const b2Vec2& position,
  Activator::Type type
) {
  uint32_t id = _id_manager->NewId();
  Activator* activator = new Activator(this, id, position, type);
  CHECK(activator != NULL);
  AddEntity(id, activator);
}

void WorldManager::CreateAlignedWall(float x, float y, Wall::Type type) {
  CHECK(_map_type == MAP_GRID);
  int xa = static_cast<int>(round(x / _block_size));
  int ya = static_cast<int>(round(y / _block_size));
  _CreateAlignedWall(xa, ya, type);
}

void WorldManager::_CreateAlignedWall(int x, int y, Wall::Type type) {
  CHECK(_map_type == MAP_GRID);
  CreateWall(b2Vec2(x * _block_size, y * _block_size), type);
}

bool WorldManager::LoadMap(const std::string& file) {
  pugi::xml_document document;
  pugi::xml_parse_result parse_result = document.load_file(file.c_str());
  if (!parse_result) {
    Error::Throw(__FILE__, __LINE__, "Unable to parse %s!\n", file.c_str());
    return false;
  }
  pugi::xml_node map_node = document.child("map");
  if (!map_node) {
    Error::Throw(__FILE__, __LINE__,
      "Tag 'map' not found in %s!\n", file.c_str());
    return false;
  }

  pugi::xml_attribute block_size = map_node.attribute("block_size");
  if (!block_size) {
    Error::Throw(__FILE__, __LINE__,
      "Tag 'map' does not have attribute 'block_size' in %s!\n", file.c_str());
    return false;
  }
  _block_size = block_size.as_float();
  _map_type = MAP_GRID;

  pugi::xml_attribute bound = map_node.attribute("bound");
  if (!bound) {
    Error::Throw(__FILE__, __LINE__,
      "Tag 'map' does not have attribute 'bound' in %s!\n", file.c_str());
    return false;
  }
  _bound = bound.as_float();

  pugi::xml_node node;
  for (node = map_node.first_child(); node; node = node.next_sibling()) {
    if (std::string(node.name()) == "wall") {
      if (!_LoadWall(node)) {
        return false;
      }
    } else if (std::string(node.name()) == "chunk") {
      if (!_LoadChunk(node)) {
        return false;
      }
    } else if (std::string(node.name()) == "spawn") {
      if (!_LoadSpawn(node)) {
        return false;
      }
    } else if (std::string(node.name()) == "kit") {
      if (!_LoadKit(node)) {
        return false;
      }
    }
  }

  return true;
}

// TODO(xairy): refactor.
bool WorldManager::_LoadWall(const pugi::xml_node& node) {
  CHECK(_map_type == MAP_GRID);
  CHECK(std::string(node.name()) == "wall");

  pugi::xml_attribute x = node.attribute("x");
  pugi::xml_attribute y = node.attribute("y");
  pugi::xml_attribute type = node.attribute("type");
  if (!x || !y || !type) {
    THROW_ERROR("Incorrect format of 'wall' in map file!\n");
    return false;
  } else {
    Wall::Type type_value;
    bool rv = _LoadWallType(type, &type_value);
    if (rv == false) {
      return false;
    }
    _CreateAlignedWall(x.as_int(), y.as_int(), type_value);
  }

  return true;
}

// TODO(xairy): refactor.
bool WorldManager::_LoadChunk(const pugi::xml_node& node) {
  CHECK(_map_type == MAP_GRID);
  CHECK(std::string(node.name()) == "chunk");

  pugi::xml_attribute x = node.attribute("x");
  pugi::xml_attribute y = node.attribute("y");
  pugi::xml_attribute width = node.attribute("width");
  pugi::xml_attribute height = node.attribute("height");
  pugi::xml_attribute type = node.attribute("type");
  if (!x || !y || !width || !height || !type) {
    THROW_ERROR("Incorrect format of 'chunk' in map file!\n");
    return false;
  } else {
    int xv = x.as_int();
    int yv = y.as_int();
    int wv = width.as_int();
    int hv = height.as_int();
    Wall::Type type_value;
    bool rv = _LoadWallType(type, &type_value);
    if (rv == false) {
      return false;
    }
    for (int i = 0; i < wv; i++) {
      for (int j = 0; j < hv; j++) {
        _CreateAlignedWall(xv + i, yv + j, type_value);
      }
    }
  }

  return true;
}

bool WorldManager::_LoadSpawn(const pugi::xml_node& node) {
  CHECK(_map_type == MAP_GRID);
  CHECK(std::string(node.name()) == "spawn");

  pugi::xml_attribute x_attr = node.attribute("x");
  pugi::xml_attribute y_attr = node.attribute("y");
  if (!x_attr || !y_attr) {
    THROW_ERROR("Incorrect format of 'spawn' in map file!\n");
    return false;
  } else {
    float x = x_attr.as_float();
    float y = y_attr.as_float();
    _spawn_positions.push_back(b2Vec2(x, y));
  }

  return true;
}

bool WorldManager::_LoadKit(const pugi::xml_node& node) {
  CHECK(_map_type == MAP_GRID);
  CHECK(std::string(node.name()) == "kit");

  pugi::xml_attribute x_attr = node.attribute("x");
  pugi::xml_attribute y_attr = node.attribute("y");
  pugi::xml_attribute hr_attr = node.attribute("health_regeneration");
  pugi::xml_attribute er_attr = node.attribute("energy_regeneration");
  pugi::xml_attribute type_attr = node.attribute("type");
  if (!x_attr || !y_attr || !hr_attr || !er_attr || !type_attr) {
    THROW_ERROR("Incorrect format of 'kit' in map file!\n");
    return false;
  } else {
    float x = x_attr.as_float();
    float y = y_attr.as_float();
    int hr = hr_attr.as_int();
    int er = er_attr.as_int();
    Kit::Type type;
    bool rv = _LoadKitType(type_attr, &type);
    if (rv == false) {
      return false;
    }
    CreateKit(b2Vec2(x, y), hr, er, type);
  }

  return true;
}

bool WorldManager::_LoadWallType(const pugi::xml_attribute& attribute,
    Wall::Type* output) {
  CHECK(std::string(attribute.name()) == "type");
  if (std::string(attribute.value()) == "ordinary") {
    *output = Wall::TYPE_ORDINARY;
  } else if (std::string(attribute.value()) == "unbreakable") {
    *output = Wall::TYPE_UNBREAKABLE;
  } else if (std::string(attribute.value()) == "morphed") {
    *output = Wall::TYPE_MORPHED;
  } else {
    THROW_ERROR("Incorrect wall type in map file!\n");
    return false;
  }
  return true;
}

bool WorldManager::_LoadKitType(const pugi::xml_attribute& attribute,
    Kit::Type* output) {
  CHECK(std::string(attribute.name()) == "type");
  if (std::string(attribute.value()) == "health") {
    *output = Kit::TYPE_HEALTH;
  } else if (std::string(attribute.value()) == "energy") {
    *output = Kit::TYPE_ENERGY;
  } else if (std::string(attribute.value()) == "composite") {
    *output = Kit::TYPE_COMPOSITE;
  } else {
    THROW_ERROR("Incorrect kit type in map file!\n");
    return false;
  }
  return true;
}

void WorldManager::MakeExplosion(const b2Vec2& location, uint32_t source_id) {
  float radius = _settings.GetFloat("player.bazooka.explosion_radius");
  int damage = _settings.GetInt32("player.bazooka.explosion_damage");

  // FIXME(xairy): can miss huge entities.
  radius += 13.0f;

  std::map<uint32_t, Entity*>::iterator i, end;
  end = _static_entities.end();
  for (i = _static_entities.begin(); i != end; ++i) {
    Entity* entity = i->second;
    float distance2 = (entity->GetPosition() - location).LengthSquared();
    if (distance2 <= radius * radius) {
      entity->Damage(damage, source_id);
    }
  }
  end = _dynamic_entities.end();
  for (i = _dynamic_entities.begin(); i != end; ++i) {
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
  _game_events.push_back(event);
}

void WorldManager::MakeSlimeExplosion(const b2Vec2& location) {
  int radius = _settings.GetInt32("player.morpher.radius");
  int lx = static_cast<int>(round(location.x / _block_size));
  int ly = static_cast<int>(round(location.y / _block_size));
  for (int x = -radius; x <= radius; x++) {
    for (int y = -radius; y <= radius; y++) {
      if (x * x + y * y <= radius * radius) {
        _CreateAlignedWall(lx + x, ly + y, Wall::TYPE_MORPHED);
      }
    }
  }
}

void WorldManager::RespawnDeadPlayers() {
  std::map<uint32_t, Entity*>::iterator i, end;
  end = _dynamic_entities.end();
  for (i = _dynamic_entities.begin(); i != end; ++i) {
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

void WorldManager::RespawnPlayer(Player* player) {
  player->SetPosition(GetRandomSpawn());
  player->RestoreHealth();
}

void WorldManager::UpdateScore(Player* player) {
  uint32_t killer_id = player->GetKillerId();
  if (killer_id == player->GetId()) {
    player->DecScore();
  } else {
    Entity* entity = GetEntity(killer_id);
    if (entity != NULL && entity->GetType() == Entity::TYPE_PLAYER) {
      Player* killer = static_cast<Player*>(entity);
      killer->IncScore();
    }
  }
}

b2Vec2 WorldManager::GetRandomSpawn() const {
  CHECK(_spawn_positions.size() > 0);
  size_t spawn_count = _spawn_positions.size();
  size_t spawn = Random(spawn_count);
  return _spawn_positions[spawn];
}

void WorldManager::OnKeyboardEvent(Player* player, const KeyboardEvent& event) {
  player->OnKeyboardEvent(event);
}

void WorldManager::OnMouseEvent(Player* player, const MouseEvent& event) {
  int bazooka_consumption =
      _settings.GetInt32("player.bazooka.energy_consumption");
  int morpher_consumption =
      _settings.GetInt32("player.morpher.energy_consumption");

  if (event.event_type == MouseEvent::EVENT_KEYDOWN &&
    event.button_type == MouseEvent::BUTTON_LEFT) {
    if (player->GetEnergy() >= bazooka_consumption) {
      player->AddEnergy(-bazooka_consumption);
      b2Vec2 start = player->GetPosition();
      b2Vec2 end(static_cast<float>(event.x), static_cast<float>(event.y));
      CreateBullet(player->GetId(), start, end, Bullet::TYPE_ROCKET);
    }
  }
  if (event.event_type == MouseEvent::EVENT_KEYDOWN &&
    event.button_type == MouseEvent::BUTTON_RIGHT) {
    if (player->GetEnergy() >= morpher_consumption) {
      player->AddEnergy(-morpher_consumption);
      b2Vec2 start = player->GetPosition();
      b2Vec2 end(static_cast<float>(event.x), static_cast<float>(event.y));
      CreateBullet(player->GetId(), start, end, Bullet::TYPE_SLIME);
    }
  }
  if (event.event_type == MouseEvent::EVENT_MOVE) {
    b2Vec2 mouse_position = b2Vec2(event.x, event.y);
    b2Vec2 direction = mouse_position - player->GetPosition();
    float angle = atan2f(-direction.x, direction.y);
    player->SetRotation(angle / M_PI * 180);
  }
}

void WorldManager::OnPlayerAction(Player* player, const PlayerAction& event) {
  if (event.type == PlayerAction::TYPE_ACTIVATE) {
    Entity* entity = GetEntity(event.target_id);
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

void WorldManager::ExplodeBullet(Bullet* bullet) {
  // We do not want 'bullet' to explode multiple times.
  if (!bullet->IsDestroyed()) {
    if (bullet->GetBulletType() == Bullet::TYPE_ROCKET) {
      MakeExplosion(bullet->GetPosition(), bullet->GetOwnerId());
    } else if (bullet->GetBulletType() == Bullet::TYPE_SLIME) {
      _morph_list.push_back(bullet->GetPosition());
    }
    bullet->Destroy();
  }
}

void WorldManager::ExplodeDummy(Dummy* dummy) {
  // We do not want 'dummy' to explode multiple times.
  if (!dummy->IsDestroyed()) {
    MakeExplosion(dummy->GetPosition(), dummy->GetId());
    dummy->Destroy();
  }
}

// Collisions.

void WorldManager::OnCollision(Activator* activator1, Activator* activator2) { }
void WorldManager::OnCollision(Activator* activator, Kit* kit) { }
void WorldManager::OnCollision(Activator* activator, Wall* wall) { }
void WorldManager::OnCollision(Activator* activator, Player* player) { }
void WorldManager::OnCollision(Activator* activator, Dummy* dummy) { }
void WorldManager::OnCollision(Activator* activator, Bullet* bullet) { }

void WorldManager::OnCollision(Kit* kit1, Kit* kit2) { }
void WorldManager::OnCollision(Kit* kit, Wall* wall) { }

void WorldManager::OnCollision(Kit* kit, Player* player) {
  player->AddHealth(kit->GetHealthRegeneration());
  player->AddEnergy(kit->GetEnergyRegeneration());
  kit->Destroy();
}

void WorldManager::OnCollision(Kit* kit, Dummy* dummy) { }
void WorldManager::OnCollision(Kit* kit, Bullet* bullet) { }

void WorldManager::OnCollision(Wall* wall1, Wall* wall2) { }
void WorldManager::OnCollision(Wall* wall, Player* player) { }

void WorldManager::OnCollision(Wall* wall, Dummy* dummy) {
  ExplodeDummy(dummy);
}

void WorldManager::OnCollision(Wall* wall, Bullet* bullet) {
  ExplodeBullet(bullet);
}

void WorldManager::OnCollision(Player* player1, Player* player2) { }

void WorldManager::OnCollision(Player* player, Dummy* dummy) {
  ExplodeDummy(dummy);
}

void WorldManager::OnCollision(Player* player, Bullet* bullet) {
  if (bullet->GetOwnerId() == player->GetId()) {
    return;
  }
  ExplodeBullet(bullet);
}

void WorldManager::OnCollision(Dummy* dummy1, Dummy* dummy2) { }

void WorldManager::OnCollision(Dummy* dummy, Bullet* bullet) {
  ExplodeBullet(bullet);
  ExplodeDummy(dummy);
}

void WorldManager::OnCollision(Bullet* bullet1, Bullet* bullet2) {
  ExplodeBullet(bullet1);
  ExplodeBullet(bullet2);
}

}  // namespace bm
