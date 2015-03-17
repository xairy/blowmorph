// Copyright (c) 2015 Blowmorph Team

#include "client/application.h"

#include <cmath>
#include <cstdio>

#include <algorithm>
#include <limits>
#include <list>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <SFML/Graphics.hpp>

#include <enet-plus/enet.h>

#include "base/error.h"
#include "base/macros.h"
#include "base/net.h"
#include "base/pstdint.h"
#include "base/time.h"
#include "base/utils.h"

#include "engine/config.h"
#include "engine/map.h"
#include "engine/protocol.h"
#include "engine/utils.h"

#include "client/contact_listener.h"
#include "client/entity.h"
#include "client/render_window.h"
#include "client/resource_manager.h"
#include "client/sprite.h"
#include "client/utils.h"

namespace bm {

Application::Application()
  : client_(NULL),
    peer_(NULL),
    event_(NULL),
    player_(NULL),
    state_(STATE_FINALIZED),
    network_state_(NETWORK_STATE_DISCONNECTED) { }

Application::~Application() {
  if (state_ != STATE_FINALIZED) {
    Finalize();
    state_ = STATE_FINALIZED;
  }
}

bool Application::Initialize() {
  CHECK(state_ == STATE_FINALIZED);

  is_running_ = false;

  if (!Config::GetInstance()->Initialize()) {
    return false;
  }

  if (!InitializeGraphics()) {
    return false;
  }

  if (!InitializePhysics()) {
    return false;
  }

  if (!InitializeNetwork()) {
    return false;
  }

  tick_rate_ = Config::GetInstance()->GetClientConfig().tick_rate;

  time_correction_ = 0;
  last_tick_ = 0;
  last_physics_simulation_ = 0;

  show_score_table_ = false;

  player_health_ = 0;
  player_energy_ = 0;

  max_player_misposition_ =
      Config::GetInstance()->GetClientConfig().max_player_misposition;
  interpolation_offset_ =
      Config::GetInstance()->GetClientConfig().interpolation_offset;

  state_ = STATE_INITIALIZED;
  return true;
}

bool Application::Run() {
  CHECK(state_ == STATE_INITIALIZED);

  if (!Connect()) {
    return false;
  }

  if (!Synchronize()) {
    return false;
  }

  // FIXME(xairy): move to a separate method.
  // FIXME(xairy): use entity_settings_.
  b2Vec2 position(client_options_.x, client_options_.y);
  Sprite* sprite = resource_manager_.CreateSprite("man");
  CHECK(sprite != NULL);
  player_ = new ClientEntity(world_.GetBox2DWorld(), client_options_.id,
    Entity::TYPE_PLAYER, "player", position, sprite);
  CHECK(player_ != NULL);
  const Config::ClientConfig& config =
    Config::GetInstance()->GetClientConfig();
  player_->EnableCaption(config.player_name, *render_window_.GetFont());
  player_->SetPosition(position);

  contact_listener_.SetPlayerId(client_options_.id);

  is_running_ = true;

  while (is_running_) {
    if (!PumpEvents()) {
      return false;
    }
    if (!PumpPackets()) {
      return false;
    }

    SimulatePhysics();
    Render();

    int64_t current_time = GetServerTime();
    if (current_time - last_tick_ > 1000.0 / tick_rate_) {
      last_tick_ = current_time;
      if (!SendInputEvents()) {
        return false;
      }
    }
  }

  return true;
}

void Application::Finalize() {
  CHECK(state_ == STATE_INITIALIZED);

  for (auto i : explosions_) {
    delete i;
  }
  explosions_.clear();

  for (auto i : *world_.GetStaticEntities()) {
    delete i.second;
  }
  world_.GetStaticEntities()->clear();

  for (auto i : *world_.GetDynamicEntities()) {
    delete i.second;
  }
  world_.GetDynamicEntities()->clear();

  if (player_ != NULL) delete player_;

  if (client_ != NULL) delete client_;
  if (event_ != NULL) delete event_;

  state_ = STATE_FINALIZED;
}

bool Application::InitializeGraphics() {
  render_window_.Initialize();

  // FIXME(xairy): receive map name from server.
  if (!map_.Load("data/maps/map.json")) {
    return false;
  }

  for (auto sprite_name : map_.GetTerrain().sprite_names) {
    Sprite* sprite = resource_manager_.CreateSprite(sprite_name);
    CHECK(sprite != NULL);
    terrain_.push_back(sprite);
  }

  float block_size = map_.GetBlockSize();
  int width = map_.GetWidth();
  int height = map_.GetHeight();

  CHECK(terrain_.size() == width * height);

  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      Sprite* sprite = terrain_[y * width + x];
      sprite->SetPosition(sf::Vector2f(x * block_size, y * block_size));
    }
  }

  return true;
}

bool Application::InitializePhysics() {
  CHECK(state_ == STATE_FINALIZED);
  world_.GetBox2DWorld()->SetContactListener(&contact_listener_);
  return true;
}

bool Application::InitializeNetwork() {
  CHECK(state_ == STATE_FINALIZED);
  CHECK(network_state_ == NETWORK_STATE_DISCONNECTED);

  if (!enet_.Initialize()) {
    return false;
  }

  std::auto_ptr<enet::ClientHost> client(enet_.CreateClientHost());
  if (client.get() == NULL) {
    return false;
  }

  std::auto_ptr<enet::Event> event(enet_.CreateEvent());
  if (event.get() == NULL) {
    return false;
  }

  client_ = client.release();
  event_ = event.release();

  network_state_ = NETWORK_STATE_INITIALIZED;
  return true;
}

bool Application::Connect() {
  CHECK(state_ == STATE_INITIALIZED);
  CHECK(network_state_ == NETWORK_STATE_INITIALIZED);

  const Config::ClientConfig& config =
    Config::GetInstance()->GetClientConfig();

  peer_ = client_->Connect(config.server_host, config.server_port);
  if (peer_ == NULL) {
    return false;
  }

  bool rv = client_->Service(event_, config.connect_timeout);
  if (rv == false) {
    return false;
  }
  if (event_->GetType() != enet::Event::TYPE_CONNECT) {
    REPORT_ERROR("Could not connect to server %s:%d.",
        config.server_host.c_str(), static_cast<int>(config.server_port));
    return false;
  }

  printf("Connected to %s:%u.\n", event_->GetPeer()->GetIp().c_str(),
      event_->GetPeer()->GetPort());

  network_state_ = NETWORK_STATE_CONNECTED;
  return true;
}

bool Application::Synchronize() {
  CHECK(state_ == STATE_INITIALIZED);
  CHECK(network_state_ == NETWORK_STATE_CONNECTED);

  printf("Synchronization started.\n");

  const Config::ClientConfig& config =
    Config::GetInstance()->GetClientConfig();

  int64_t sync_timeout = config.sync_timeout;
  int64_t start_time = Timestamp();

  // Send login data.

  CHECK(config.player_name.size() <= LoginData::MAX_LOGIN_LENGTH);
  LoginData login_data;
  std::copy(config.player_name.begin(), config.player_name.end(),
      &login_data.login[0]);
  login_data.login[config.player_name.size()] = '\0';
  bool rv = SendPacket(peer_, Packet::TYPE_LOGIN, login_data, true);
  if (rv == false) {
    return false;
  }

  printf("Login data sent, login: %s.\n", login_data.login);

  // Receive client options.

  while (true) {
    int64_t time = Timestamp();
    if (time - start_time > sync_timeout) {
      REPORT_ERROR("Synchronization failed: time's out.");
      return false;
    }

    uint32_t service_timeout =
        static_cast<uint32_t>(sync_timeout - (time - start_time));
    bool rv = client_->Service(event_, service_timeout);
    if (rv == false) {
      return false;
    }
    if (event_->GetType() != enet::Event::TYPE_RECEIVE) {
      continue;
    }

    std::vector<char> buffer;
    event_->GetData(&buffer);

    Packet::Type type;
    rv = ExtractPacketType(buffer, &type);
    if (rv == false) {
      REPORT_ERROR("Incorrect client options packet format.");
      return false;
    }
    if (type != Packet::TYPE_CLIENT_OPTIONS) {
      continue;
    }

    rv = ExtractPacketData<Packet::Type, ClientOptions>(
            buffer, &client_options_);
    if (rv == false) {
      REPORT_ERROR("Incorrect client options packet format.");
      return false;
    }

    break;
  }

  printf("Client options received.\n");

  // Send a time synchronization request.

  TimeSyncData request_data;
  request_data.client_time = Timestamp();

  rv = SendPacket(peer_, Packet::TYPE_SYNC_TIME_REQUEST, request_data, true);
  if (rv == false) {
    return false;
  }

  // Receive a time synchronization response.

  while (true) {
    int64_t time = Timestamp();
    if (time - start_time > sync_timeout) {
      REPORT_ERROR("Synchronization failed: time's out.");
      return false;
    }

    uint32_t service_timeout =
        static_cast<uint32_t>(sync_timeout - (time - start_time));
    bool rv = client_->Service(event_, service_timeout);
    if (rv == false) {
      return false;
    }
    if (event_->GetType() != enet::Event::TYPE_RECEIVE) {
      continue;
    }

    std::vector<char> message;
    event_->GetData(&message);
    const Packet::Type* type = reinterpret_cast<Packet::Type*>(&message[0]);
    if (*type != Packet::TYPE_SYNC_TIME_RESPONSE) {
      continue;
    }
    const TimeSyncData* response_data =
        reinterpret_cast<const TimeSyncData*>(&message[0] + sizeof(*type));

    // Calculate the time correction.
    int64_t client_time = Timestamp();
    latency_ = (client_time - response_data->client_time) / 2;
    time_correction_ = response_data->server_time + latency_ - client_time;

    break;
  }

  printf("Synchronized time, latency: %d ms.\n", static_cast<int>(latency_));

  // Notify the server that the client has synchronized.

  ClientStatus client_status;
  client_status.status = ClientStatus::STATUS_SYNCHRONIZED;

  rv = SendPacket(peer_, Packet::TYPE_CLIENT_STATUS, client_status, true);
  if (rv == false) {
    return false;
  }

  printf("Synchronization completed.\n");

  network_state_ = NETWORK_STATE_LOGGED_IN;
  return true;
}

int64_t Application::GetServerTime() {
  CHECK(state_ == STATE_INITIALIZED);
  CHECK(network_state_ == NETWORK_STATE_LOGGED_IN);
  return Timestamp() + time_correction_;
}

bool Application::PumpEvents() {
  CHECK(state_ == STATE_INITIALIZED);
  sf::Event event;
  while (render_window_.PollEvent(&event)) {
    if (!ProcessEvent(event)) {
      return false;
    }
  }
  return true;
}

bool Application::ProcessEvent(const sf::Event& event) {
  CHECK(state_ == STATE_INITIALIZED);

  switch (event.type) {
    case sf::Event::Closed:
      return OnQuitEvent();
    case sf::Event::KeyPressed:
    case sf::Event::KeyReleased:
      return OnKeyEvent(event);
    case sf::Event::MouseMoved:
      break;
    case sf::Event::MouseButtonPressed:
    case sf::Event::MouseButtonReleased:
      return OnMouseButtonEvent(event);
    default:
      break;
  }

  return true;
}

bool Application::OnQuitEvent() {
  CHECK(state_ == STATE_INITIALIZED);

  is_running_ = false;

  const Config::ClientConfig& config =
    Config::GetInstance()->GetClientConfig();

  if (!DisconnectPeer(peer_, event_, client_, config.connect_timeout)) {
    REPORT_ERROR("Didn't receive EVENT_DISCONNECT event while disconnecting.");
    return false;
  } else {
    printf("Disconnected.\n");
  }

  return true;
}

bool Application::OnMouseButtonEvent(const sf::Event& event) {
  CHECK(state_ == STATE_INITIALIZED);

  MouseEvent mouse_event;
  mouse_event.time = GetServerTime();

  if (event.type == sf::Event::MouseButtonReleased) {
    mouse_event.event_type = MouseEvent::EVENT_KEYUP;
  } else if (event.type == sf::Event::MouseButtonPressed) {
    mouse_event.event_type = MouseEvent::EVENT_KEYDOWN;
  } else {
    CHECK(false);
  }

  switch (event.mouseButton.button) {
    case sf::Mouse::Left:
      mouse_event.button_type = MouseEvent::BUTTON_LEFT;
      break;
    case sf::Mouse::Right:
      mouse_event.button_type = MouseEvent::BUTTON_RIGHT;
      break;
    case sf::Mouse::Middle:
      if (event.type == sf::Event::MouseButtonPressed) {
        show_score_table_ = true;
      }
      if (event.type == sf::Event::MouseButtonReleased) {
        show_score_table_ = false;
      }
      return true;
    default:
      return true;
  }

  int screen_width = render_window_.GetWindowSize().x;
  int screen_height = render_window_.GetWindowSize().y;
  mouse_event.x = (event.mouseButton.x - screen_width / 2) +
    player_->GetPosition().x;
  mouse_event.y = (event.mouseButton.y - screen_height / 2) +
    player_->GetPosition().y;

  mouse_events_.push_back(mouse_event);

  return true;
}

bool Application::OnKeyEvent(const sf::Event& event) {
  CHECK(state_ == STATE_INITIALIZED);

  KeyboardEvent keyboard_event;
  keyboard_event.time = GetServerTime();

  bool pressed;

  if (event.type == sf::Event::KeyReleased) {
    keyboard_event.event_type = KeyboardEvent::EVENT_KEYUP;
    pressed = false;
  } else if (event.type == sf::Event::KeyPressed) {
    keyboard_event.event_type = KeyboardEvent::EVENT_KEYDOWN;
    pressed = true;
  } else {
    CHECK(false);
  }

  switch (event.key.code) {
    case sf::Keyboard::A:
      keyboard_state_.left = pressed;
      keyboard_event.key_type = KeyboardEvent::KEY_LEFT;
      break;
    case sf::Keyboard::D:
      keyboard_state_.right = pressed;
      keyboard_event.key_type = KeyboardEvent::KEY_RIGHT;
      break;
    case sf::Keyboard::W:
      keyboard_state_.up = pressed;
      keyboard_event.key_type = KeyboardEvent::KEY_UP;
      break;
    case sf::Keyboard::S:
      keyboard_state_.down = pressed;
      keyboard_event.key_type = KeyboardEvent::KEY_DOWN;
      break;
    case sf::Keyboard::E:
      if (event.type == sf::Event::KeyPressed) {
        if (!OnActivateAction()) {
          return false;
        }
      }
      return true;
    case sf::Keyboard::Escape:
      OnQuitEvent();
      return true;
    case sf::Keyboard::Tab:
    case sf::Keyboard::Unknown:
      // Check for 'sf::Keyboard::Unknown' due to a bug in SFML which
      // causes 'key.code' to be 'Unknown' when Shift+Tab is pressed.
      if (event.type == sf::Event::KeyPressed) {
        show_score_table_ = true;
      }
      if (event.type == sf::Event::KeyReleased) {
        show_score_table_ = false;
      }
      return true;
    default:
      return true;
  }

  keyboard_events_.push_back(keyboard_event);

  return true;
}

bool Application::PumpPackets() {
  CHECK(state_ == STATE_INITIALIZED);
  CHECK(network_state_ == NETWORK_STATE_LOGGED_IN);

  std::vector<char> buffer;

  do {
    bool rv = client_->Service(event_, 0);
    if (rv == false) {
      return false;
    }

    switch (event_->GetType()) {
      case enet::Event::TYPE_RECEIVE: {
        event_->GetData(&buffer);
        bool rv = ProcessPacket(buffer);
        if (rv == false) {
          return false;
        }
      } break;

      case enet::Event::TYPE_CONNECT: {
        REPORT_WARNING("Got EVENT_CONNECT while being already connected.");
      } break;

      case enet::Event::TYPE_DISCONNECT: {
        network_state_ = NETWORK_STATE_DISCONNECTED;
        REPORT_ERROR("Connection lost.");
        return false;
      } break;

      case enet::Event::TYPE_NONE:
        break;
    }
  } while (event_->GetType() != enet::Event::TYPE_NONE);

  return true;
}

bool Application::ProcessPacket(const std::vector<char>& buffer) {
  CHECK(state_ == STATE_INITIALIZED);
  CHECK(network_state_ == NETWORK_STATE_LOGGED_IN);

  Packet::Type type;
  bool rv = ExtractPacketType(buffer, &type);
  if (rv == false) {
    REPORT_ERROR("Incorrect packet format!");
    return false;
  }

  switch (type) {
    case Packet::TYPE_ENTITY_APPEARED:
    case Packet::TYPE_ENTITY_UPDATED: {
      EntitySnapshot snapshot;
      bool rv = ExtractPacketData<Packet::Type, EntitySnapshot>(
                  buffer, &snapshot);
      if (rv == false) {
        REPORT_ERROR("Incorrect entity packet format!");
        return false;
      }
      if (snapshot.type == EntitySnapshot::ENTITY_TYPE_PLAYER) {
        player_scores_[snapshot.id] = static_cast<int>(snapshot.data[2]);
      }
      if (snapshot.id == player_->GetId()) {
        OnPlayerUpdate(&snapshot);
        break;
      }
      if (world_.GetEntity(snapshot.id) != NULL) {
        OnEntityUpdate(&snapshot);
      } else {
        OnEntityAppearance(&snapshot);
      }
    } break;

    case Packet::TYPE_GAME_EVENT: {
      GameEvent event;
      bool rv = ExtractPacketData<Packet::Type, GameEvent>(buffer, &event);
      if (rv == false) {
        REPORT_ERROR("Incorrect game event packet format!");
        return false;
      }
      if (event.type == GameEvent::TYPE_EXPLOSION) {
        Sprite* explosion = resource_manager_.CreateSprite("explosion");
        if (explosion == NULL) {
          return false;
        }
        explosion->SetPosition(Round(sf::Vector2f(event.x, event.y)));
        explosion->Play();
        explosions_.push_back(explosion);
      } else if (event.type == GameEvent::TYPE_ENTITY_DISAPPEARED) {
        if (event.entity.type == EntitySnapshot::ENTITY_TYPE_PLAYER) {
          player_scores_.erase(event.entity.id);
        }
        if (!OnEntityDisappearance(&event.entity)) {
          return false;
        }
      }
    } break;

    case Packet::TYPE_PLAYER_INFO: {
      PlayerInfo player_info;
      bool rv = ExtractPacketData<Packet::Type, PlayerInfo>(
                  buffer, &player_info);
      if (rv == false) {
        REPORT_ERROR("Incorrect player info packet format!");
        return false;
      }

      std::string player_name(player_info.login);
      player_names_[player_info.id] = player_name;
      ClientEntity* entity = static_cast<ClientEntity*>(
          world_.GetEntity(player_info.id));
      if (entity != NULL) {
        entity->EnableCaption(player_name, *render_window_.GetFont());
      }
    } break;

    default:
      break;
  }

  return true;
}

void Application::OnEntityAppearance(const EntitySnapshot* snapshot) {
  CHECK(state_ == STATE_INITIALIZED);
  CHECK(snapshot != NULL);

  uint32_t id = snapshot->id;
  b2Vec2 position = b2Vec2(snapshot->x, snapshot->y);
  std::string entity_name = std::string(&snapshot->name[0]);

  Entity::Type type;
  std::string sprite_name;

  switch (snapshot->type) {
    case EntitySnapshot::ENTITY_TYPE_ACTIVATOR: {
      type = Entity::TYPE_ACTIVATOR;
      sprite_name = Config::GetInstance()->
        GetActivatorsConfig().at(entity_name).sprite_name;
    } break;

    case EntitySnapshot::ENTITY_TYPE_CRITTER: {
      type = Entity::TYPE_CRITTER;
      sprite_name = Config::GetInstance()->
        GetCrittersConfig().at(entity_name).sprite_name;
    } break;

    case EntitySnapshot::ENTITY_TYPE_KIT: {
      type = Entity::TYPE_KIT;
      sprite_name = Config::GetInstance()->
        GetKitsConfig().at(entity_name).sprite_name;
    } break;

    case EntitySnapshot::ENTITY_TYPE_WALL: {
      type = Entity::TYPE_WALL;
      sprite_name = Config::GetInstance()->
        GetWallsConfig().at(entity_name).sprite_name;
    } break;

    case EntitySnapshot::ENTITY_TYPE_PROJECTILE: {
      type = Entity::TYPE_PROJECTILE;
      sprite_name = Config::GetInstance()->
        GetProjectilesConfig().at(entity_name).sprite_name;
    } break;

    case EntitySnapshot::ENTITY_TYPE_PLAYER: {
      type = Entity::TYPE_PLAYER;
      sprite_name = Config::GetInstance()->
        GetPlayersConfig().at(entity_name).sprite_name;
    } break;

    default:
      CHECK(false);  // Unreachable.
  }

  Sprite* sprite = resource_manager_.CreateSprite(sprite_name);
  CHECK(sprite != NULL);

  ClientEntity* entity = new ClientEntity(world_.GetBox2DWorld(),
      id, type, entity_name, position, sprite);
  CHECK(entity != NULL);

  entity->SetRotation(snapshot->angle);
  world_.AddEntity(id, entity);
}

void Application::OnEntityUpdate(const EntitySnapshot* snapshot) {
  CHECK(state_ == STATE_INITIALIZED);
  CHECK(snapshot != NULL);

  b2Vec2 position = b2Vec2(snapshot->x, snapshot->y);

  ClientEntity* entity = static_cast<ClientEntity*>(
      world_.GetEntity(snapshot->id));
  CHECK(entity != NULL);

  // FIXME(xairy): add is_static flag.
  if (snapshot->type == EntitySnapshot::ENTITY_TYPE_ACTIVATOR ||
      snapshot->type == EntitySnapshot::ENTITY_TYPE_KIT ||
      snapshot->type == EntitySnapshot::ENTITY_TYPE_WALL) {
    CHECK(entity->IsStatic() == true);
    entity->SetPosition(position);
    entity->SetRotation(snapshot->angle);
  } else {
    CHECK(entity->IsStatic() == false);
    int64_t server_time = GetServerTime();
    if (server_time - interpolation_offset_ >= snapshot->time) {
      // Ignore snapshots that are too old.
      return;
    }
    entity->SetInterpolationPosition(position, snapshot->time,
        interpolation_offset_, server_time);
    entity->SetRotation(snapshot->angle);
    // TODO(xairy): use SetInterpolationRotation.
    // dynamic_entities_[snapshot->id]->SetInterpolationRotation(
    //   snapshot->angle, snapshot->time, interpolation_offset_, server_time);
  }
}

void Application::OnPlayerUpdate(const EntitySnapshot* snapshot) {
  CHECK(state_ == STATE_INITIALIZED);
  CHECK(snapshot != NULL);

  player_health_ = snapshot->data[0];
  player_energy_ = snapshot->data[1];

  b2Vec2 position = b2Vec2(snapshot->x, snapshot->y);
  b2Vec2 distance = player_->GetPosition() - position;

  if (Length(distance) > max_player_misposition_) {
    player_->SetPosition(position);
  }
}

bool Application::OnEntityDisappearance(const EntitySnapshot* snapshot) {
  CHECK(state_ == STATE_INITIALIZED);
  CHECK(snapshot != NULL);

  auto i = world_.GetDynamicEntities()->find(snapshot->id);
  if (i != world_.GetDynamicEntities()->end()) {
    delete i->second;
    world_.RemoveEntity(i->first);
  }

  i = world_.GetStaticEntities()->find(snapshot->id);
  if (i != world_.GetStaticEntities()->end()) {
    delete i->second;
    world_.RemoveEntity(i->first);
  }

  return true;
}

void Application::SimulatePhysics() {
  CHECK(state_ == STATE_INITIALIZED);

  if (network_state_ == NETWORK_STATE_LOGGED_IN) {
    int64_t current_time = Timestamp();
    int64_t delta_time = current_time - last_physics_simulation_;
    last_physics_simulation_ = current_time;

    b2Vec2 velocity(0.0f, 0.0f);
    velocity.x = keyboard_state_.left * (-client_options_.speed)
      + keyboard_state_.right * (client_options_.speed);
    velocity.y = keyboard_state_.up * (-client_options_.speed)
      + keyboard_state_.down * (client_options_.speed);
    player_->SetImpulse(player_->GetMass() * velocity);

    int32_t velocity_iterations = 6;
    int32_t position_iterations = 2;
    world_.GetBox2DWorld()->Step(static_cast<float>(delta_time) / 1000,
      velocity_iterations, position_iterations);
  }
}

void Application::Render() {
  CHECK(state_ == STATE_INITIALIZED);

  render_window_.StartFrame();

  if (network_state_ == NETWORK_STATE_LOGGED_IN) {
    b2Vec2 position = player_->GetPosition();
    render_window_.SetViewCenter(sf::Vector2f(position.x, position.y));

    render_window_.RenderSprites(terrain_);

    // FIXME(xairy): madness.
    std::list<Sprite*>::iterator it2;
    for (it2 = explosions_.begin(); it2 != explosions_.end();) {
      render_window_.RenderSprite(*it2);
      if ((*it2)->IsStopped()) {
        std::list<Sprite*>::iterator it1 = it2;
        ++it1;
        delete *it2;
        explosions_.erase(it2);
        it2 = it1;
      } else {
        ++it2;
      }
    }

    render_window_.RenderWorld(&world_);

    // Set player rotation.
    b2Vec2 mouse_position = GetMousePosition();
    b2Vec2 direction = mouse_position - player_->GetPosition();
    float angle = atan2f(-direction.x, direction.y);
    player_->SetRotation(angle);
    render_window_.RenderEntity(player_);

    render_window_.RenderPlayerStats(player_health_, client_options_.max_health,
                              player_energy_, client_options_.energy_capacity);
    render_window_.RenderMinimap(&world_, player_);

    if (show_score_table_) {
      render_window_.RenderScoretable(player_scores_, player_names_);
    }
  }

  render_window_.EndFrame();
}

bool Application::SendInputEvents() {
  for (size_t i = 0; i < keyboard_events_.size(); i++) {
    bool rv = SendPacket(peer_, Packet::TYPE_KEYBOARD_EVENT,
        keyboard_events_[i]);
    if (rv == false) {
      return false;
    }
  }
  keyboard_events_.clear();

  // TODO(xairy): make mouse event actions.
  for (size_t i = 0; i < mouse_events_.size(); i++) {
    bool rv = SendPacket(peer_, Packet::TYPE_MOUSE_EVENT,
        mouse_events_[i]);
    if (rv == false) {
      return false;
    }
  }
  mouse_events_.clear();

  // Send mouse position.
  MouseEvent event;
  event.time = GetServerTime();
  event.button_type =  MouseEvent::BUTTON_NONE;
  event.event_type = MouseEvent::EVENT_MOVE;
  b2Vec2 mouse_position = GetMousePosition();
  event.x = mouse_position.x;
  event.y = mouse_position.y;
  bool rv = SendPacket(peer_, Packet::TYPE_MOUSE_EVENT, event);
  if (rv == false) {
    return false;
  }

  return true;
}

bool Application::OnActivateAction() {
  b2Body* b = RayCast(world_.GetBox2DWorld(), player_->GetPosition(),
    GetMousePosition());
  if (b == NULL) {
    return true;
  }

  Entity* entity = static_cast<Entity*>(b->GetUserData());
  PlayerAction action;
  action.type = PlayerAction::TYPE_ACTIVATE;
  action.target_id = entity->GetId();

  bool rv = SendPacket(peer_, Packet::TYPE_PLAYER_ACTION, action);
  if (rv == false) {
    return false;
  }

  printf("! Activate: %u %d\n", entity->GetId(),
    static_cast<int>(entity->GetType()));
  return true;
}

b2Vec2 Application::GetMousePosition() const {
  sf::Vector2i mouse_position = render_window_.GetMousePosition();
  int screen_width = render_window_.GetWindowSize().x;
  int screen_height = render_window_.GetWindowSize().y;
  mouse_position.x = (mouse_position.x - screen_width / 2) +
    player_->GetPosition().x;
  mouse_position.y = (mouse_position.y - screen_height / 2) +
    player_->GetPosition().y;
  return b2Vec2(mouse_position.x, mouse_position.y);
}

}  // namespace bm
