// Copyright (c) 2013 Blowmorph Team

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

#include "client/object.h"
#include "client/resource_manager.h"
#include "client/sprite.h"

namespace {

float Length(const sf::Vector2f& vector) {
  return sqrt(vector.x * vector.x + vector.y * vector.y);
}

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

  if (!settings_.Open("data/client.cfg")) {
    return false;
  }

  if (!InitializeGraphics()) {
    return false;
  }

  if (!InitializeNetwork()) {
    return false;
  }

  tick_rate_ = settings_.GetInt32("client.tick_rate");

  time_correction_ = 0;
  last_tick_ = 0;
  last_physics_simulation_ = 0;

  player_health_ = 0;
  player_blow_charge_ = 0;
  player_morph_charge_ = 0;
  
  show_score_table_ = 0;

  wall_size_ = 16;
  player_size_ = 30;

  max_player_misposition_ = settings_.GetFloat("client.max_player_misposition");
  interpolation_offset_ = settings_.GetInt64("client.interpolation_offset");

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

  // XXX(alex): maybe we should have a xml file for each object with
  //            texture paths, pivots, captions, etc
  // FIXME(xairy): move to a separate method.
  sf::Vector2f player_pos(client_options_.x, client_options_.y);
  Sprite* sprite = resource_manager_.CreateSprite("mechos");
  CHECK(sprite != NULL);
  player_ = new Object(client_options_.id, EntitySnapshot::ENTITY_TYPE_PLAYER,
      sprite, player_pos, 0);
  CHECK(player_ != NULL);
  player_->ShowCaption(settings_.GetString("player.login"), *font_);
  player_->SetPosition(player_pos);

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

  std::map<uint32_t, Object*>::iterator it;
  for (it = walls_.begin(); it != walls_.end(); ++it) {
    delete it->second;
  }
  walls_.clear();

  for (it = objects_.begin(); it != objects_.end(); ++it) {
    delete it->second;
  }
  objects_.clear();

  if (player_ != NULL) delete player_;

  if (client_ != NULL) delete client_;
  if (event_ != NULL) delete event_;

  if (font_ != NULL) delete font_;
  if (render_window_ != NULL) delete render_window_;

  state_ = STATE_FINALIZED;
}

bool Application::InitializeGraphics() {
  CHECK(state_ == STATE_FINALIZED);

  uint32_t width = settings_.GetUInt32("resolution.width");
  uint32_t height = settings_.GetUInt32("resolution.height");

  sf::VideoMode video_mode(width, height);
  render_window_ = new sf::RenderWindow(video_mode, "Blowmorph");
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

  std::string host = settings_.GetString("server.host");
  uint16_t port = settings_.GetUInt16("server.port");

  peer_ = client_->Connect(host, port);
  if (peer_ == NULL) {
    return false;
  }

  uint32_t connect_timeout = settings_.GetUInt32("client.connect_timeout");

  bool rv = client_->Service(event_, connect_timeout);
  if (rv == false) {
    return false;
  }
  if (event_->GetType() != enet::Event::TYPE_CONNECT) {
    THROW_ERROR("Could not connect to server.");
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

  int64_t sync_timeout = settings_.GetInt64("client.sync_timeout");
  int64_t start_time = Timestamp();

  // Send login data.

  std::string login = settings_.GetString("player.login");
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

  //printf("Synchronized time, latency: %ld ms.\n", latency_);

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

  uint32_t connect_timeout = settings_.GetUInt32("client.connect_timeout");

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

  if (event.mouseButton.button == sf::Mouse::Left) {
    mouse_event.button_type = MouseEvent::BUTTON_LEFT;
  } else {
    mouse_event.button_type = MouseEvent::BUTTON_RIGHT;
  }

  if (event.type == sf::Event::MouseButtonReleased) {
    mouse_event.event_type = MouseEvent::EVENT_KEYUP;
  } else if (event.type == sf::Event::MouseButtonPressed) {
    mouse_event.event_type = MouseEvent::EVENT_KEYDOWN;
  } else {
    CHECK(false);
  }

  int screenWidth = render_window_->getSize().x;
  int screenHeight = render_window_->getSize().y;
  mouse_event.x = (event.mouseButton.x - screenWidth / 2) +
    player_->GetPosition().x;
  mouse_event.y = (event.mouseButton.y - screenHeight / 2) +
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
    case sf::Keyboard::Escape:
      OnQuitEvent();
      break;
    default:
      break;
  }
  //Press Tab to show score table
  if (event.key.code == sf::Keyboard::Tab) {
    if (event.type == sf::Event::KeyPressed)
      show_score_table_ = 1;
    if (event.type == sf::Event::KeyReleased)
      show_score_table_ = 0; 
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
        player_scores_[snapshot.id] = static_cast<int>(snapshot.data[3]); 
      }
      if (snapshot.id == player_->id) {
        OnPlayerUpdate(&snapshot);
        break;
      }

      if (objects_.count(snapshot.id) > 0 ||
          walls_.count(snapshot.id) > 0) {
        OnEntityUpdate(&snapshot);
      } else {
        OnEntityAppearance(&snapshot);
      }
      }
      break;

    case Packet::TYPE_ENTITY_DISAPPEARED: {
      EntitySnapshot snapshot;
      bool rv = ExtractPacketData(buffer, &snapshot);
      if (rv == false) {
        THROW_ERROR("Incorrect entity packet format!");
        return false;
      }
      if (snapshot.type == EntitySnapshot::ENTITY_TYPE_PLAYER) {
        player_scores_.erase(snapshot.id); 
      }
      if (!OnEntityDisappearance(&snapshot)) {
        return false;
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
      if (objects_.count(player_info.id) == 1) {
        objects_[player_info.id]->ShowCaption(player_name, *font_);
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
  sf::Vector2f position = sf::Vector2f(snapshot->x, snapshot->y);

  switch (snapshot->type) {
    case EntitySnapshot::ENTITY_TYPE_WALL: {
      std::string sprite_id;
      if (snapshot->data[0] == EntitySnapshot::WALL_TYPE_ORDINARY) {
        sprite_id = "ordinary_wall";
      } else if (snapshot->data[0] == EntitySnapshot::WALL_TYPE_UNBREAKABLE) {
        sprite_id = "unbreakable_wall";
      } else if (snapshot->data[0] == EntitySnapshot::WALL_TYPE_MORPHED) {
        sprite_id = "morphed_wall";
      } else {
        CHECK(false);  // Unreachable.
      }
      Sprite* sprite = resource_manager_.CreateSprite(sprite_id);
      CHECK(sprite != NULL);
      Object* wall = new Object(id, type, sprite, position, time);
      CHECK(wall != NULL);
      walls_[id] = wall;
    } break;

    case EntitySnapshot::ENTITY_TYPE_BULLET: {
      Sprite* sprite = resource_manager_.CreateSprite("bullet");
      CHECK(sprite != NULL);
      Object* object = new Object(id, type, sprite, position, time);
      CHECK(object != NULL);
      object->EnableInterpolation(interpolation_offset_);
      object->SetPosition(position);
      objects_[id] = object;
    } break;

    case EntitySnapshot::ENTITY_TYPE_PLAYER: {
      Sprite* sprite = resource_manager_.CreateSprite("mechos");
      CHECK(sprite != NULL);
      Object* object = new Object(id, type, sprite, position, time);
      CHECK(object != NULL);
      object->EnableInterpolation(interpolation_offset_);
      object->SetPosition(position);
      if (player_names_.count(id) == 1) {
        object->ShowCaption(player_names_[id], *font_);
      }
      objects_[id] = object;
    } break;

    case EntitySnapshot::ENTITY_TYPE_DUMMY: {
      Sprite* sprite = resource_manager_.CreateSprite("dummy");
      CHECK(sprite != NULL);
      Object* object = new Object(id, type, sprite, position, time);
      CHECK(object != NULL);
      object->EnableInterpolation(interpolation_offset_);
      object->SetPosition(position);
      objects_[id] = object;
    } break;

    case EntitySnapshot::ENTITY_TYPE_STATION: {
      std::string sprite_id;
      switch (snapshot->data[0]) {
        case EntitySnapshot::STATION_TYPE_HEALTH: {
          sprite_id = "health_kit";
        } break;
        case EntitySnapshot::STATION_TYPE_BLOW: {
          sprite_id = "blow_kit";
        } break;
        case EntitySnapshot::STATION_TYPE_MORPH: {
          sprite_id = "morph_kit";
        } break;
        case EntitySnapshot::STATION_TYPE_COMPOSITE: {
          sprite_id = "composite_kit";
        } break;
        default: {
          CHECK(false);  // Unreachable.
        }
      }
      Sprite* sprite = resource_manager_.CreateSprite(sprite_id);
      CHECK(sprite != NULL);
      Object* object = new Object(id, type, sprite, position, time);
      CHECK(object != NULL);
      object->EnableInterpolation(interpolation_offset_);
      object->SetPosition(position);
      objects_[id] = object;
    } break;

    default:
      break;
  }
}

void Application::OnEntityUpdate(const EntitySnapshot* snapshot) {
  CHECK(state_ == STATE_INITIALIZED);
  CHECK(snapshot != NULL);

  int64_t time = snapshot->time;
  sf::Vector2f position = sf::Vector2f(snapshot->x, snapshot->y);

  ObjectState state;
  state.position = position;
  state.health = static_cast<float>(snapshot->data[0]);
  state.blow_charge = static_cast<float>(snapshot->data[1]);
  state.morph_charge = static_cast<float>(snapshot->data[2]);

  if (snapshot->type == EntitySnapshot::ENTITY_TYPE_WALL) {
    walls_[snapshot->id]->PushState(state, time);
  } else {
    objects_[snapshot->id]->PushState(state, time);
  }
}

void Application::OnPlayerUpdate(const EntitySnapshot* snapshot) {
  CHECK(state_ == STATE_INITIALIZED);
  CHECK(snapshot != NULL);

  sf::Vector2f position = sf::Vector2f(snapshot->x, snapshot->y);
  sf::Vector2f distance = player_->GetPosition() - position;

  ObjectState state;
  state.position = position;
  player_health_ = static_cast<float>(snapshot->data[0]);
  player_blow_charge_ = static_cast<float>(snapshot->data[1]);
  player_morph_charge_ = static_cast<float>(snapshot->data[2]);

  if (Length(distance) > max_player_misposition_) {
    player_->EnforceState(state, snapshot->time);
  } else {
    // player_->UpdateCurrentState(state, snapshot->time);
  }
}

bool Application::OnEntityDisappearance(const EntitySnapshot* snapshot) {
  CHECK(state_ == STATE_INITIALIZED);
  CHECK(snapshot != NULL);

  typedef std::map<uint32_t, Object*>::iterator It;
  It it = objects_.find(snapshot->id);
  if (it != objects_.end()) {
    delete it->second;
    objects_.erase(it);
  }
  it = walls_.find(snapshot->id);
  if (it != walls_.end()) {
    delete it->second;
    walls_.erase(it);
  }

  if (snapshot->type == EntitySnapshot::ENTITY_TYPE_BULLET) {
    // TODO(xairy): create explosion animation on explosion packet.
    Sprite* explosion = resource_manager_.CreateSprite("explosion");
    if (explosion == NULL) {
      return false;
    }
    explosion->SetPosition(Round(sf::Vector2f(snapshot->x, snapshot->y)));
    explosion->Play();
    explosions_.push_back(explosion);
  }

  return true;
}

void Application::SimulatePhysics() {
  CHECK(state_ == STATE_INITIALIZED);

  if (network_state_ == NETWORK_STATE_LOGGED_IN) {
    int64_t current_time = GetServerTime();
    int64_t delta_time = current_time - last_physics_simulation_;
    last_physics_simulation_ = current_time;

    float delta_x = (keyboard_state_.right - keyboard_state_.left) *
      client_options_.speed * delta_time;
    float delta_y = (keyboard_state_.down - keyboard_state_.up) *
      client_options_.speed * delta_time;

    bool intersection_x = false;
    bool intersection_y = false;

    std::map<uint32_t, Object*>::iterator it;
    sf::Vector2f player_pos = player_->GetPosition(current_time);

    for (it = walls_.begin() ; it != walls_.end(); it++) {
      sf::Vector2f wall_pos = it->second->GetPosition(current_time);
      float player_to_wall_x = abs(wall_pos.x - (player_pos.x + delta_x));
      float player_to_wall_y = abs(wall_pos.y - player_pos.y);
      if (player_to_wall_x < (player_size_ + wall_size_) / 2 &&
          player_to_wall_y < (player_size_ + wall_size_) / 2) {
        intersection_x = true;
        break;
      }
    }
    for (it = walls_.begin() ; it != walls_.end(); it++) {
      sf::Vector2f wall_pos = it->second->GetPosition(current_time);
      float player_to_wall_x = abs(wall_pos.x - player_pos.x);
      float player_to_wall_y = abs(wall_pos.y - (player_pos.y + delta_y));
      if (player_to_wall_x < (player_size_ + wall_size_) / 2 &&
          player_to_wall_y < (player_size_ + wall_size_) / 2) {
        intersection_y = true;
        break;
      }
    }

    if (!intersection_x) player_->Move(sf::Vector2f(delta_x, 0.0f));
    if (!intersection_y) player_->Move(sf::Vector2f(0.0f, delta_y));
  }
}

void Application::Render() {
  CHECK(state_ == STATE_INITIALIZED);

  render_window_->clear();

  if (network_state_ == NETWORK_STATE_LOGGED_IN) {
    int64_t render_time = GetServerTime();
    sf::Vector2f player_pos = player_->GetPosition(render_time);

    view_.setCenter(Round(player_pos));
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

    std::map<uint32_t, Object*>::iterator it;
    for (it = walls_.begin() ; it != walls_.end(); ++it) {
      RenderObject(it->second, render_time, font_, *render_window_);
    }
    for (it = objects_.begin() ; it != objects_.end(); ++it) {
      RenderObject(it->second, render_time, font_, *render_window_);
    }

    RenderObject(player_, render_time, font_, *render_window_);

    RenderHUD();
  }

  render_window_->display();
}

void Application::WriteText(const std::string& text, int x, int y) {
// TODO (use render_window_ for rendering).
  
  sf::Text new_text;
  new_text.setString(text);
  new_text.setCharacterSize(50);
  new_text.setColor(sf::Color::Red);
  new_text.setStyle(sf::Text::Regular);  
  new_text.setFont(*font_);
  
  sf::Vector2f size = view_.getSize();  
  sf::Transform text_pos;
  text_pos.translate(view_.getCenter() - size / 2.0f);
  text_pos.translate(x, y);
  render_window_->draw(new_text, text_pos);  
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

  // Draw health, blow and morph bars.

  int32_t max_health = client_options_.max_health;
  float health_rect_width = 200.0f * player_health_ / max_health;
  float health_rect_height = 10.0f;
  sf::Vector2f health_rect_size(health_rect_width, health_rect_height);
  sf::RectangleShape health_rect;
  health_rect.setSize(health_rect_size);
  health_rect.setPosition(sf::Vector2f(20.0f, -70.0f));
  health_rect.setFillColor(sf::Color(0xFF, 0x00, 0xFF, 0xBB));
  render_window_->draw(health_rect, bottom_left_transform);

  int32_t blow_capacity = client_options_.blow_capacity;
  float blow_rect_width = 200.0f * player_blow_charge_ / blow_capacity;
  float blow_rect_height = 10.0f;
  sf::Vector2f blow_rect_size(blow_rect_width, blow_rect_height);
  sf::RectangleShape blow_charge_rect;
  blow_charge_rect.setSize(blow_rect_size);
  blow_charge_rect.setPosition(sf::Vector2f(20.0f, -50.0f));
  blow_charge_rect.setFillColor(sf::Color(0x00, 0xFF, 0xFF, 0xBB));
  render_window_->draw(blow_charge_rect, bottom_left_transform);

  int32_t morph_capacity = client_options_.morph_capacity;
  float morph_rect_width = 200.0f * player_morph_charge_ / morph_capacity;
  float morph_rect_height = 10.0f;
  sf::Vector2f morph_rect_size(morph_rect_width, morph_rect_height);
  sf::RectangleShape morph_charge_rect;
  morph_charge_rect.setSize(morph_rect_size);
  morph_charge_rect.setPosition(sf::Vector2f(20.0f, -30.0f));
  morph_charge_rect.setFillColor(sf::Color(0xFF, 0xFF, 0x00, 0xBB));
  render_window_->draw(morph_charge_rect, bottom_left_transform);

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

  std::map<uint32_t, Object*>::const_iterator it;
  for (it = objects_.begin(); it != objects_.end(); ++it) {
    Object* obj = it->second;

    sf::Vector2f obj_pos = obj->GetPosition(render_time);
    sf::Vector2f player_pos = player_->GetPosition(render_time);
    sf::Vector2f rel = obj_pos - player_pos;
    if (Length(rel) < compass_range) {
      rel = rel * (compass_radius / compass_range);
      sf::CircleShape circle(1.0f);
      circle.setPosition(compass_center + rel);
      circle.setFillColor(sf::Color(0xFF, 0x00, 0x00, 0xFF));
      render_window_->draw(circle, top_right_transform);
    }
  }

  for (it = walls_.begin(); it != walls_.end(); ++it) {
    Object* obj = it->second;

    sf::Vector2f obj_pos = obj->GetPosition(render_time);
    sf::Vector2f player_pos = player_->GetPosition(render_time);
    sf::Vector2f rel = obj_pos - player_pos;
    if (Length(rel) < compass_range) {
      rel = rel * (compass_radius / compass_range);
      sf::CircleShape circle(1.0f);
      circle.setPosition(compass_center + rel);
      circle.setFillColor(sf::Color(0x00, 0xFF, 0x00, 0xFF));
      render_window_->draw(circle, top_right_transform);
    }
  }

  sf::CircleShape circle(1.0f);
  circle.setPosition(compass_center);
  circle.setFillColor(sf::Color(0x00, 0x00, 0xFF, 0xFF));
  render_window_->draw(circle, top_right_transform);
  
  // Draw scores
  
  if (show_score_table_) {
  std::map<uint32_t, int>::iterator it;
  int i = 0;
  for (it = player_scores_.begin(); it != player_scores_.end(); ++it) {
    std::stringstream type;
    type << it->second;
	std::string score = type.str();
    WriteText(player_names_[it->first] + " " + score, 0, 50 * i);
    i++;
  }
  }
}

// Sends input events to the server and
// clears the input event queues afterwards.
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

  return true;
}

}  // namespace bm
