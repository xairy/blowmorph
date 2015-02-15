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
#include "base/protocol.h"
#include "base/pstdint.h"
#include "base/settings_manager.h"
#include "base/time.h"
#include "base/utils.h"

#include "client/contact_listener.h"
#include "client/entity.h"
#include "client/resource_manager.h"
#include "client/sprite.h"

namespace {

// TODO(xairy): move to sfml_utils.
sf::Vector2f Round(const sf::Vector2f& vector) {
  return sf::Vector2f(floor(vector.x), floor(vector.y));
}

}  // anonymous namespace

namespace bm {

Application::Application()
  : render_window_(NULL),
    font_(NULL),
    client_(NULL),
    peer_(NULL),
    event_(NULL),
    player_(NULL),
    world_(NULL),
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

  if (!client_settings_.Open("data/client.cfg")) {
    return false;
  }

  if (!body_settings_.Open("data/bodies.cfg")) {
    return false;
  }

  if (!activator_settings_.Open("data/activators.cfg")) {
    return false;
  }
  if (!critter_settings_.Open("data/critters.cfg")) {
    return false;
  }
  if (!kit_settings_.Open("data/kits.cfg")) {
    return false;
  }
  if (!player_settings_.Open("data/players.cfg")) {
    return false;
  }
  if (!projectile_settings_.Open("data/projectiles.cfg")) {
    return false;
  }
  if (!wall_settings_.Open("data/walls.cfg")) {
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

  tick_rate_ = client_settings_.GetInt32("client.tick_rate");

  time_correction_ = 0;
  last_tick_ = 0;
  last_physics_simulation_ = 0;

  show_score_table_ = false;

  player_health_ = 0;
  player_energy_ = 0;

  max_player_misposition_ =
      client_settings_.GetFloat("client.max_player_misposition");
  interpolation_offset_ =
      client_settings_.GetInt64("client.interpolation_offset");

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
  player_ = new Entity(&body_settings_, "man", client_options_.id,
    Entity::TYPE_PLAYER, world_, sprite, position, 0);
  CHECK(player_ != NULL);
  player_->EnableCaption(client_settings_.GetString("player.login"), *font_);
  player_->GetBody()->SetPosition(position);

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

  std::list<Sprite*>::iterator it2;
  for (it2 = explosions_.begin(); it2 != explosions_.end(); ++it2) {
    delete *it2;
  }
  explosions_.clear();

  std::map<uint32_t, Entity*>::iterator it;
  for (it = static_entities_.begin(); it != static_entities_.end(); ++it) {
    delete it->second;
  }
  static_entities_.clear();

  for (it = dynamic_entities_.begin(); it != dynamic_entities_.end(); ++it) {
    delete it->second;
  }
  dynamic_entities_.clear();

  if (player_ != NULL) delete player_;

  if (client_ != NULL) delete client_;
  if (event_ != NULL) delete event_;

  if (font_ != NULL) delete font_;
  if (render_window_ != NULL) delete render_window_;

  if (world_ != NULL) delete world_;

  state_ = STATE_FINALIZED;
}

bool Application::InitializeGraphics() {
  CHECK(state_ == STATE_FINALIZED);

  uint32_t width = client_settings_.GetUInt32("graphics.width");
  uint32_t height = client_settings_.GetUInt32("graphics.height");
  bool fullscreen = client_settings_.GetBool("graphics.fullscreen");

  sf::VideoMode video_mode(width, height);
  sf::Uint32 style = fullscreen ? sf::Style::Fullscreen : sf::Style::Default;
  render_window_ = new sf::RenderWindow(video_mode, "Blowmorph", style);
  CHECK(render_window_ != NULL);
  view_.reset(sf::FloatRect(0.0f, 0.0f,
      static_cast<float>(width), static_cast<float>(height)));
  render_window_->setView(view_);

  // By default if a key is held, multiple 'KeyPressed' events
  // will be generated. We disable such behaviour.
  render_window_->setKeyRepeatEnabled(false);

  font_ = new sf::Font();
  CHECK(font_ != NULL);
  font_->loadFromFile("data/fonts/tahoma.ttf");

  return true;
}

bool Application::InitializePhysics() {
  CHECK(state_ == STATE_FINALIZED);

  world_ = new b2World(b2Vec2(0.0f, 0.0f));
  CHECK(world_ != NULL);
  world_->SetContactListener(&contact_listener_);

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

  std::string host = client_settings_.GetString("server.host");
  uint16_t port = client_settings_.GetUInt16("server.port");

  peer_ = client_->Connect(host, port);
  if (peer_ == NULL) {
    return false;
  }

  uint32_t connect_timeout =
      client_settings_.GetUInt32("client.connect_timeout");

  bool rv = client_->Service(event_, connect_timeout);
  if (rv == false) {
    return false;
  }
  if (event_->GetType() != enet::Event::TYPE_CONNECT) {
    THROW_ERROR("Could not connect to server %s:%d.",
        host.c_str(), static_cast<int>(port));
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

  int64_t sync_timeout = client_settings_.GetInt64("client.sync_timeout");
  int64_t start_time = Timestamp();

  // Send login data.

  std::string login = client_settings_.GetString("player.login");
  CHECK(login.size() <= LoginData::MAX_LOGIN_LENGTH);

  LoginData login_data;
  std::copy(login.begin(), login.end(), &login_data.login[0]);
  login_data.login[login.size()] = '\0';

  bool rv = SendPacket(peer_, Packet::TYPE_LOGIN, login_data, true);
  if (rv == false) {
    return false;
  }

  printf("Login data sent, login: %s.\n", login_data.login);

  // Receive client options.

  while (true) {
    int64_t time = Timestamp();
    if (time - start_time > sync_timeout) {
      THROW_ERROR("Synchronization failed: time's out.");
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
      THROW_ERROR("Incorrect client options packet format.");
      return false;
    }
    if (type != Packet::TYPE_CLIENT_OPTIONS) {
      continue;
    }

    rv = ExtractPacketData(buffer, &client_options_);
    if (rv == false) {
      THROW_ERROR("Incorrect client options packet format.");
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
      THROW_ERROR("Synchronization failed: time's out.");
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
  while (render_window_->pollEvent(event)) {
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

  uint32_t connect_timeout =
      client_settings_.GetUInt32("client.connect_timeout");

  if (!DisconnectPeer(peer_, event_, client_, connect_timeout)) {
    THROW_ERROR("Didn't receive EVENT_DISCONNECT event while disconnecting.");
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

  int screen_width = render_window_->getSize().x;
  int screen_height = render_window_->getSize().y;
  mouse_event.x = (event.mouseButton.x - screen_width / 2) +
    player_->GetBody()->GetPosition().x;
  mouse_event.y = (event.mouseButton.y - screen_height / 2) +
    player_->GetBody()->GetPosition().y;

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
        THROW_WARNING("Got EVENT_CONNECT while being already connected.");
      } break;

      case enet::Event::TYPE_DISCONNECT: {
        network_state_ = NETWORK_STATE_DISCONNECTED;
        THROW_ERROR("Connection lost.");
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
    THROW_ERROR("Incorrect packet format!");
    return false;
  }

  switch (type) {
    case Packet::TYPE_ENTITY_APPEARED:
    case Packet::TYPE_ENTITY_UPDATED: {
      EntitySnapshot snapshot;
      bool rv = ExtractPacketData(buffer, &snapshot);
      if (rv == false) {
        THROW_ERROR("Incorrect entity packet format!");
        return false;
      }
      if (snapshot.type == EntitySnapshot::ENTITY_TYPE_PLAYER) {
        player_scores_[snapshot.id] = static_cast<int>(snapshot.data[2]);
      }
      if (snapshot.id == player_->GetId()) {
        OnPlayerUpdate(&snapshot);
        break;
      }
      if (dynamic_entities_.count(snapshot.id) > 0 ||
          static_entities_.count(snapshot.id) > 0) {
        OnEntityUpdate(&snapshot);
      } else {
        OnEntityAppearance(&snapshot);
      }
    } break;

    case Packet::TYPE_GAME_EVENT: {
      GameEvent event;
      bool rv = ExtractPacketData(buffer, &event);
      if (rv == false) {
        THROW_ERROR("Incorrect game event packet format!");
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
      bool rv = ExtractPacketData(buffer, &player_info);
      if (rv == false) {
        THROW_ERROR("Incorrect player info packet format!");
        return false;
      }

      std::string player_name(player_info.login);
      player_names_[player_info.id] = player_name;
      if (dynamic_entities_.count(player_info.id) == 1) {
        dynamic_entities_[player_info.id]->EnableCaption(player_name, *font_);
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

  int64_t time = snapshot->time;
  uint32_t id = snapshot->id;
  EntitySnapshot::EntityType type = snapshot->type;
  b2Vec2 position = b2Vec2(snapshot->x, snapshot->y);

  std::string entity_config;
  SettingsManager* entity_settings = NULL;

  switch (snapshot->type) {
    case EntitySnapshot::ENTITY_TYPE_WALL: {
      if (snapshot->data[0] == EntitySnapshot::WALL_TYPE_ORDINARY) {
        entity_config = "ordinary_wall";
      } else if (snapshot->data[0] == EntitySnapshot::WALL_TYPE_UNBREAKABLE) {
        entity_config = "unbreakable_wall";
      } else if (snapshot->data[0] == EntitySnapshot::WALL_TYPE_MORPHED) {
        entity_config = "morphed_wall";
      } else {
        CHECK(false);  // Unreachable.
      }
      entity_settings = &wall_settings_;
    } break;

    case EntitySnapshot::ENTITY_TYPE_PROJECTILE: {
      switch (snapshot->data[0]) {
        case EntitySnapshot::PROJECTILE_TYPE_ROCKET: {
          entity_config = "rocket";
        } break;
        case EntitySnapshot::PROJECTILE_TYPE_SLIME: {
          entity_config = "slime";
        } break;
        default: {
          CHECK(false);  // Unreachable.
        }
      }
      entity_settings = &projectile_settings_;
    } break;

    case EntitySnapshot::ENTITY_TYPE_PLAYER: {
      entity_config = "player";
      entity_settings = &player_settings_;
    } break;

    case EntitySnapshot::ENTITY_TYPE_CRITTER: {
      entity_config = "zombie";
      entity_settings = &critter_settings_;
    } break;

    case EntitySnapshot::ENTITY_TYPE_KIT: {
      switch (snapshot->data[0]) {
        case EntitySnapshot::KIT_TYPE_HEALTH: {
          entity_config = "health_kit";
        } break;
        case EntitySnapshot::KIT_TYPE_ENERGY: {
          entity_config = "energy_kit";
        } break;
        case EntitySnapshot::KIT_TYPE_COMPOSITE: {
          entity_config = "composite_kit";
        } break;
        default: {
          CHECK(false);  // Unreachable.
        }
      }
      entity_settings = &kit_settings_;
    } break;

    case EntitySnapshot::ENTITY_TYPE_ACTIVATOR: {
      entity_config = "door";
      entity_settings = &activator_settings_;
    } break;

    default:
      CHECK(false);  // Unreachable.
  }

  std::string sprite_config =
      entity_settings->GetString(entity_config + ".sprite");

  Sprite* sprite = resource_manager_.CreateSprite(sprite_config);
  CHECK(sprite != NULL);

  std::string body_config =
      entity_settings->GetString(entity_config + ".body");

  switch (snapshot->type) {
    case EntitySnapshot::ENTITY_TYPE_WALL: {
      Entity* wall = new Entity(&body_settings_, body_config, id,
        Entity::TYPE_WALL, world_, sprite, position, time);
      CHECK(wall != NULL);
      static_entities_[id] = wall;
    } break;

    case EntitySnapshot::ENTITY_TYPE_PROJECTILE: {
      Entity* object = new Entity(&body_settings_, body_config, id,
        Entity::TYPE_PROJECTILE, world_, sprite, position, time);
      CHECK(object != NULL);
      dynamic_entities_[id] = object;
    } break;

    case EntitySnapshot::ENTITY_TYPE_PLAYER: {
      Entity* object = new Entity(&body_settings_, body_config, id,
        Entity::TYPE_PLAYER, world_, sprite, position, time);
      CHECK(object != NULL);
      if (player_names_.count(id) == 1) {
        object->EnableCaption(player_names_[id], *font_);
      }
      dynamic_entities_[id] = object;
    } break;

    case EntitySnapshot::ENTITY_TYPE_CRITTER: {
      Entity* object = new Entity(&body_settings_, body_config, id,
        Entity::TYPE_CRITTER, world_, sprite, position, time);
      CHECK(object != NULL);
      dynamic_entities_[id] = object;
    } break;

    case EntitySnapshot::ENTITY_TYPE_KIT: {
      Entity* object = new Entity(&body_settings_, body_config, id,
        Entity::TYPE_KIT, world_, sprite, position, time);
      CHECK(object != NULL);
      dynamic_entities_[id] = object;
    } break;

    case EntitySnapshot::ENTITY_TYPE_ACTIVATOR: {
      Entity* object = new Entity(&body_settings_, body_config, id,
        Entity::TYPE_ACTIVATOR, world_, sprite, position, time);
      CHECK(object != NULL);
      static_entities_[id] = object;
    } break;

    default:
      break;
  }
}

void Application::OnEntityUpdate(const EntitySnapshot* snapshot) {
  CHECK(state_ == STATE_INITIALIZED);
  CHECK(snapshot != NULL);

  b2Vec2 position = b2Vec2(snapshot->x, snapshot->y);

  // FIXME(xairy): add is_static flag.
  if (snapshot->type == EntitySnapshot::ENTITY_TYPE_WALL ||
      snapshot->type == EntitySnapshot::ENTITY_TYPE_ACTIVATOR) {
    static_entities_[snapshot->id]->GetBody()->SetPosition(position);
  } else {
    int64_t server_time = GetServerTime();
    if (server_time - interpolation_offset_ >= snapshot->time) {
      // Ignore snapshots that are too old.
      return;
    }
    CHECK(dynamic_entities_.count(snapshot->id) == 1);
    dynamic_entities_[snapshot->id]->SetInterpolationPosition(position,
        snapshot->time, interpolation_offset_, server_time);
    // FIXME(xairy): ignoring angle on entity appearance.
    dynamic_entities_[snapshot->id]->GetBody()->SetRotation(snapshot->angle);
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
  b2Vec2 distance = player_->GetBody()->GetPosition() - position;

  if (Length(distance) > max_player_misposition_) {
    player_->GetBody()->SetPosition(position);
  }
}

bool Application::OnEntityDisappearance(const EntitySnapshot* snapshot) {
  CHECK(state_ == STATE_INITIALIZED);
  CHECK(snapshot != NULL);

  typedef std::map<uint32_t, Entity*>::iterator It;
  It it = dynamic_entities_.find(snapshot->id);
  if (it != dynamic_entities_.end()) {
    delete it->second;
    dynamic_entities_.erase(it);
  }
  it = static_entities_.find(snapshot->id);
  if (it != static_entities_.end()) {
    delete it->second;
    static_entities_.erase(it);
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
    player_->GetBody()->SetImpulse(player_->GetBody()->GetMass() * velocity);

    int32_t velocity_iterations = 6;
    int32_t position_iterations = 2;
    world_->Step(static_cast<float>(delta_time) / 1000,
      velocity_iterations, position_iterations);
  }
}

void Application::Render() {
  CHECK(state_ == STATE_INITIALIZED);

  render_window_->clear();

  if (network_state_ == NETWORK_STATE_LOGGED_IN) {
    int64_t render_time = GetServerTime();
    b2Vec2 position = player_->GetBody()->GetPosition();

    view_.setCenter(Round(sf::Vector2f(position.x, position.y)));
    render_window_->setView(view_);

    // FIXME(xairy): madness.
    std::list<Sprite*>::iterator it2;
    for (it2 = explosions_.begin(); it2 != explosions_.end();) {
      (*it2)->Render(render_window_);
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

    std::map<uint32_t, Entity*>::iterator it;
    for (it = static_entities_.begin() ; it != static_entities_.end(); ++it) {
      it->second->Render(render_window_, render_time);
    }
    for (it = dynamic_entities_.begin() ; it != dynamic_entities_.end(); ++it) {
      it->second->Render(render_window_, render_time);
    }

    // Set player rotation.
    b2Vec2 mouse_position = GetMousePosition();
    b2Vec2 direction = mouse_position - player_->GetBody()->GetPosition();
    float angle = atan2f(-direction.x, direction.y);
    player_->GetBody()->SetRotation(angle / M_PI * 180);

    player_->Render(render_window_, render_time);

    RenderHUD();
  }

  render_window_->display();
}

// TODO(xairy): load HUD layout parameters from some config file.
void Application::RenderHUD() {
  CHECK(state_ == STATE_INITIALIZED);

  // Initialize transforms for drawing in different corners of the screen.

  sf::Vector2f size = view_.getSize();

  sf::Transform top_left_transform;
  top_left_transform.translate(view_.getCenter() - size / 2.0f);

  size.x = -size.x;
  sf::Transform top_right_transform;
  top_right_transform.translate(view_.getCenter() - size / 2.0f);

  size.y = -size.y;
  sf::Transform bottom_right_transform;
  bottom_right_transform.translate(view_.getCenter() - size / 2.0f);

  size.x = -size.x;
  sf::Transform bottom_left_transform;
  bottom_left_transform.translate(view_.getCenter() - size / 2.0f);

  // Draw health and energy bars.

  int32_t max_health = client_options_.max_health;
  float health_rect_width = 200.0f * player_health_ / max_health;
  float health_rect_height = 10.0f;
  sf::Vector2f health_rect_size(health_rect_width, health_rect_height);
  sf::RectangleShape health_rect;
  health_rect.setSize(health_rect_size);
  health_rect.setPosition(sf::Vector2f(20.0f, -50.0f));
  health_rect.setFillColor(sf::Color(0xFF, 0x00, 0xFF, 0xBB));
  render_window_->draw(health_rect, bottom_left_transform);

  int32_t energy_capacity = client_options_.energy_capacity;
  float energy_rect_width = 200.0f * player_energy_ / energy_capacity;
  float energy_rect_height = 10.0f;
  sf::Vector2f energy_rect_size(energy_rect_width, energy_rect_height);
  sf::RectangleShape energy_charge_rect;
  energy_charge_rect.setSize(energy_rect_size);
  energy_charge_rect.setPosition(sf::Vector2f(20.0f, -30.0f));
  energy_charge_rect.setFillColor(sf::Color(0x00, 0xFF, 0xFF, 0xBB));
  render_window_->draw(energy_charge_rect, bottom_left_transform);

  // Draw gun slots.

  sf::Vector2f gun_slot_rect_size(30.0f, 30.0f);
  sf::RectangleShape gun_slot_rect;
  gun_slot_rect.setSize(gun_slot_rect_size);
  gun_slot_rect.setPosition(sf::Vector2f(-50.0f, -50.0f));
  gun_slot_rect.setFillColor(sf::Color(0xFF, 0xFF, 0xFF, 0x00));
  gun_slot_rect.setOutlineColor(sf::Color(0xFF, 0x00, 0x00, 0xBB));
  gun_slot_rect.setOutlineThickness(3.0f);
  render_window_->draw(gun_slot_rect, bottom_right_transform);

  gun_slot_rect.setPosition(sf::Vector2f(-110.0f, -50.0f));
  gun_slot_rect.setFillColor(sf::Color(0xFF, 0xFF, 0xFF, 0x00));
  gun_slot_rect.setOutlineColor(sf::Color(0x00, 0xFF, 0x00, 0xBB));
  gun_slot_rect.setOutlineThickness(3.0f);
  render_window_->draw(gun_slot_rect, bottom_right_transform);

  // Draw compass.

  float compass_range = 400.0f;
  float compass_radius = 60.0f;
  sf::Vector2f compass_center(-80.0f, 80.0f);

  sf::CircleShape compass_border(compass_radius);
  compass_border.setPosition(compass_center.x - compass_radius,
      compass_center.y - compass_radius);  // Left top corner, not center.
  compass_border.setOutlineColor(sf::Color(0xFF, 0xFF, 0xFF, 0xFF));
  compass_border.setOutlineThickness(1.0f);
  compass_border.setFillColor(sf::Color(0xFF, 0xFF, 0xFF, 0x00));
  render_window_->draw(compass_border, top_right_transform);

  int64_t render_time = GetServerTime();

  std::map<uint32_t, Entity*>::const_iterator it;
  for (it = dynamic_entities_.begin(); it != dynamic_entities_.end(); ++it) {
    Entity* obj = it->second;

    b2Vec2 obj_pos = obj->GetBody()->GetPosition();
    b2Vec2 player_pos = player_->GetBody()->GetPosition();
    b2Vec2 rel = obj_pos - player_pos;
    if (Length(rel) < compass_range) {
      rel = (compass_radius / compass_range) * rel;
      sf::CircleShape circle(1.0f);
      circle.setPosition(compass_center + sf::Vector2f(rel.x, rel.y));
      circle.setFillColor(sf::Color(0xFF, 0x00, 0x00, 0xFF));
      render_window_->draw(circle, top_right_transform);
    }
  }

  for (it = static_entities_.begin(); it != static_entities_.end(); ++it) {
    Entity* obj = it->second;

    b2Vec2 obj_pos = obj->GetBody()->GetPosition();
    b2Vec2 player_pos = player_->GetBody()->GetPosition();
    b2Vec2 rel = obj_pos - player_pos;
    if (Length(rel) < compass_range) {
      rel = (compass_radius / compass_range) * rel;
      sf::CircleShape circle(1.0f);
      circle.setPosition(compass_center + sf::Vector2f(rel.x, rel.y));
      circle.setFillColor(sf::Color(0x00, 0xFF, 0x00, 0xFF));
      render_window_->draw(circle, top_right_transform);
    }
  }

  sf::CircleShape circle(1.0f);
  circle.setPosition(compass_center);
  circle.setFillColor(sf::Color(0x00, 0x00, 0xFF, 0xFF));
  render_window_->draw(circle, top_right_transform);

  // Draw score table.

  if (show_score_table_) {
    // TODO(xairy): sort players by score.

    sf::Vector2f view_size = view_.getSize();
    size_t player_count = player_scores_.size();
    sf::Vector2f score_rect_size(400.0f, player_count * 50.0f);
    sf::Vector2f score_rect_position((view_size.x - score_rect_size.x) / 2,
          (view_size.y - score_rect_size.y) / 2);
    sf::RectangleShape score_rect;
    score_rect.setSize(score_rect_size);
    score_rect.setPosition(score_rect_position);
    score_rect.setFillColor(sf::Color(0xAA, 0xAA, 0xAA, 0xBB));
    render_window_->draw(score_rect, top_left_transform);

    int i = 0;
    std::map<uint32_t, int>::iterator it;
    for (it = player_scores_.begin(); it != player_scores_.end(); ++it) {
      std::string score = IntToStr(it->second);
      WriteText(player_names_[it->first], score_rect_position +
          sf::Vector2f(20.0f, 50.0f * i), 40, sf::Color::Blue);
      WriteText(score, score_rect_position +
          sf::Vector2f(score_rect_size.x - 50.0f, 50.0f * i),
          40, sf::Color::Magenta);
      i++;
    }
  }
}

void Application::WriteText(const std::string& str,
    const sf::Vector2f& position, int size, sf::Color color) {
  sf::Text text;
  text.setString(str);
  text.setCharacterSize(size);
  text.setColor(color);
  text.setStyle(sf::Text::Regular);
  text.setFont(*font_);
  sf::Transform transform;
  transform.translate(view_.getCenter() - view_.getSize() / 2.0f);
  transform.translate(position);
  render_window_->draw(text, transform);
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
  b2Body* b = RayCast(world_, player_->GetBody()->GetPosition(),
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
  sf::Vector2i mouse_position = sf::Mouse::getPosition(*render_window_);
  int screen_width = render_window_->getSize().x;
  int screen_height = render_window_->getSize().y;
  mouse_position.x = (mouse_position.x - screen_width / 2) +
    player_->GetBody()->GetPosition().x;
  mouse_position.y = (mouse_position.y - screen_height / 2) +
    player_->GetBody()->GetPosition().y;
  return b2Vec2(mouse_position.x, mouse_position.y);
}

}  // namespace bm
