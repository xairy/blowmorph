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

#include <enet-plus/enet.hpp>

#include "base/error.h"
#include "base/macros.h"
#include "base/protocol.h"
#include "base/pstdint.h"
#include "base/settings_manager.h"

#include "client/object.h"
#include "client/sprite.h"

#include "client/sys.h"
#include "client/net.h"

namespace {

float Length(const sf::Vector2f& vector) {
  return sqrt(vector.x * vector.x + vector.y * vector.y);
}

sf::Vector2f Round(const sf::Vector2f& vector) {
  return sf::Vector2f(round(vector.x), round(vector.y));
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
    client_options_(NULL),
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

  sync_timeout_ = 1000;
  connect_timeout_ = 500;

  if (!InitializeNetwork()) {
    return false;
  }

  tick_rate_ = 30;

  time_correction_ = 0;
  last_tick_ = 0;
  last_physics_simulation_ = 0;

  player_health_ = 0;
  player_blow_charge_ = 0;
  player_morph_charge_ = 0;

  wall_size_ = 16;
  player_size_ = 30;

  max_player_misposition_ = 25.0f;

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
  sf::Vector2f player_pos(client_options_->x, client_options_->y);
  player_ = new Object(player_pos, 0, client_options_->id,
      EntitySnapshot::ENTITY_TYPE_PLAYER, "data/sprites/mechos.sprite",
      latency_);
  CHECK(player_ != NULL);
  // XXX(alex): maybe we should have a xml file for each object with
  //            texture paths, pivots, captions, etc
  player_->SetPosition(player_pos);
  player_->visible = true;
  player_->name_visible = true;

  is_running_ = true;

  while (is_running_) {
    if (!PumpEvents()) {
      return false;
    }
    if (!PumpPackets(0)) {
      return false;
    }
    SimulatePhysics();
    Render();

    int64_t current_time = GetServerTime();
    if (current_time - last_tick_ > 1000.0 / tick_rate_) {
      last_tick_ = current_time;
      net::SendInputEvents(peer_, keyboard_events_, mouse_events_);
    }
  }

  return true;
}

void Application::Finalize() {
  CHECK(state_ == STATE_INITIALIZED);

  if (client_options_ != NULL) delete client_options_;

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
  view_.reset(sf::FloatRect(0, 0, width, height));
  render_window_->setView(view_);

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

  bool rv = client_->Service(event_, connect_timeout_);
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

  int64_t start_time = sys::Timestamp();

  while (true) {
    int64_t time = sys::Timestamp();
    if (time - start_time > sync_timeout_) {
      THROW_ERROR("Could not synchronize with server.");
      return false;
    }

    uint32_t service_timeout = (sync_timeout_ - (time - start_time));
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
    if (*type != Packet::TYPE_CLIENT_OPTIONS) {
      continue;
    }
    const ClientOptions* client_options =
        reinterpret_cast<const ClientOptions*>(&message[0] + sizeof(*type));

    client_options_ = new ClientOptions(*client_options);

    break;
  }

  // Send a time synchronization request.
  TimeSyncData request_data;
  request_data.client_time = sys::Timestamp();

  std::vector<char> buffer;
  net::AppendPacketToBuffer(buffer, &request_data,
      Packet::TYPE_SYNC_TIME_REQUEST);

  bool rv = peer_->Send(&buffer[0], buffer.size(), true);
  if (rv == false) {
    return false;
  }

  while (true) {
    int64_t time = sys::Timestamp();
    if (time - start_time > sync_timeout_) {
      THROW_ERROR("Could not synchronize with server.");
      return false;
    }

    uint32_t service_timeout = (sync_timeout_ - (time - start_time));
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
    int64_t client_time = sys::Timestamp();
    latency_ = (client_time - response_data->client_time) / 2;
    time_correction_ = response_data->server_time + latency_ - client_time;

    break;
  }

  printf("Synchronized, latency: %ld ms.\n", latency_);

  network_state_ = NETWORK_STATE_LOGGED_IN;
  return true;
}

int64_t Application::GetServerTime() {
  CHECK(state_ == STATE_INITIALIZED);
  CHECK(network_state_ == NETWORK_STATE_LOGGED_IN);
  return sys::Timestamp() + time_correction_;
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

  if (!net::DisconnectPeer(peer_, event_, client_, connect_timeout_)) {
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

  keyboard_events_.push_back(keyboard_event);

  return true;
}

bool Application::PumpPackets(uint32_t timeout) {
  CHECK(state_ == STATE_INITIALIZED);
  CHECK(network_state_ == NETWORK_STATE_LOGGED_IN);

  std::vector<char> message;

  int64_t start_time = sys::Timestamp();
  do {
    int64_t time = sys::Timestamp();
    CHECK(time >= start_time);

    // If we have run out of time, break and return
    // unless timeout is zero.
    if (time - start_time > timeout && timeout != 0) {
      break;
    }

    uint32_t service_timeout = (timeout == 0) ?
        0 : (timeout - (time - start_time));
    bool rv = client_->Service(event_, service_timeout);
    if (rv == false) {
      return false;
    }

    switch (event_->GetType()) {
      case enet::Event::TYPE_RECEIVE: {
        event_->GetData(&message);

        const Packet::Type* type =
            reinterpret_cast<Packet::Type*>(&message[0]);
        const void* data =
            reinterpret_cast<void*>(&message[0] + sizeof(Packet::Type));
        size_t len = message.size() - sizeof(Packet::Type);

        // FIXME(alex): will only process single packet at a time
        ProcessPacket(*type, data, len);
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

bool Application::ProcessPacket(Packet::Type type,
    const void* data, size_t len) {
  CHECK(state_ == STATE_INITIALIZED);
  CHECK(network_state_ == NETWORK_STATE_LOGGED_IN);

  bool isSnapshot = len == sizeof(EntitySnapshot) &&
                    (type == Packet::TYPE_ENTITY_APPEARED ||
                     type == Packet::TYPE_ENTITY_DISAPPEARED ||
                     type == Packet::TYPE_ENTITY_UPDATED);

  // FIXME(xairy): simplify.
  if (isSnapshot) {
    const EntitySnapshot* snapshot =
        reinterpret_cast<const EntitySnapshot*>(data);
    if (type == Packet::TYPE_ENTITY_DISAPPEARED) {
      if (!OnEntityDisappearance(snapshot)) {
        return false;
      }
    } else {
      if (snapshot->id == player_->id) {
        OnPlayerUpdate(snapshot);
        return true;
      }

      if (objects_.count(snapshot->id) > 0 ||
          walls_.count(snapshot->id) > 0) {
        OnEntityUpdate(snapshot);
      } else {
        OnEntityAppearance(snapshot);
      }
    }
  } else {
    THROW_WARNING("Received packet is not an entity snapshot.");
  }

  return true;
}

void Application::OnEntityAppearance(const EntitySnapshot* snapshot) {
  CHECK(state_ == STATE_INITIALIZED);
  CHECK(snapshot != NULL);

  int64_t time = snapshot->time;
  sf::Vector2f position = sf::Vector2f(snapshot->x, snapshot->y);

  switch (snapshot->type) {
    case EntitySnapshot::ENTITY_TYPE_WALL: {
      size_t tile;
      if (snapshot->data[0] == EntitySnapshot::WALL_TYPE_ORDINARY) {
        tile = 3;
      } else if (snapshot->data[0] == EntitySnapshot::WALL_TYPE_UNBREAKABLE) {
        tile = 2;
      } else if (snapshot->data[0] == EntitySnapshot::WALL_TYPE_MORPHED) {
        tile = 1;
      }
      walls_[snapshot->id] = new Object(position, time, snapshot->id,
          snapshot->type, "data/sprites/wall.sprite", latency_);
      walls_[snapshot->id]->sprite.SetCurrentFrame(tile);
      walls_[snapshot->id]->EnableInterpolation();
      walls_[snapshot->id]->visible = true;
      walls_[snapshot->id]->name_visible = false;
    } break;

    case EntitySnapshot::ENTITY_TYPE_BULLET: {
      objects_[snapshot->id] = new Object(position, time, snapshot->id,
          snapshot->type, "data/sprites/bullet.sprite", latency_);
      objects_[snapshot->id]->EnableInterpolation();
      objects_[snapshot->id]->visible = true;
      objects_[snapshot->id]->name_visible = false;
    } break;

    case EntitySnapshot::ENTITY_TYPE_PLAYER: {
      objects_[snapshot->id] = new Object(position, time, snapshot->id,
          snapshot->type, "data/sprites/mechos.sprite", latency_);
      objects_[snapshot->id]->EnableInterpolation();
      objects_[snapshot->id]->visible = true;
      objects_[snapshot->id]->name_visible = true;
    } break;

    case EntitySnapshot::ENTITY_TYPE_DUMMY: {
      objects_[snapshot->id] = new Object(position, time, snapshot->id,
          snapshot->type, "data/sprites/dummy.sprite", latency_);
      objects_[snapshot->id]->EnableInterpolation();
      objects_[snapshot->id]->visible = true;
      objects_[snapshot->id]->name_visible = false;
    } break;

    case EntitySnapshot::ENTITY_TYPE_STATION: {
      size_t tile;
      switch (snapshot->data[0]) {
        case EntitySnapshot::STATION_TYPE_HEALTH: {
          tile = 0;
        } break;
        case EntitySnapshot::STATION_TYPE_BLOW: {
          tile = 2;
        } break;
        case EntitySnapshot::STATION_TYPE_MORPH: {
          tile = 1;
        } break;
        case EntitySnapshot::STATION_TYPE_COMPOSITE: {
          tile = 3;
        } break;
        default: {
          CHECK(false);  // Unreachable.
        }
      }
      objects_[snapshot->id] = new Object(position, time, snapshot->id,
          snapshot->type, "data/sprites/station.sprite", latency_);
      objects_[snapshot->id]->sprite.SetCurrentFrame(tile);
      objects_[snapshot->id]->EnableInterpolation();
      objects_[snapshot->id]->visible = true;
      objects_[snapshot->id]->name_visible = false;
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
  state.blowCharge = static_cast<float>(snapshot->data[1]);
  state.morphCharge = static_cast<float>(snapshot->data[2]);

  if (snapshot->type == EntitySnapshot::ENTITY_TYPE_WALL) {
    walls_[snapshot->id]->UpdateState(state, time);
  } else {
    objects_[snapshot->id]->UpdateState(state, time);
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
    Sprite* explosion = new Sprite();
    CHECK(explosion != NULL);
    bool rv = explosion->Initialize("data/sprites/explosion.sprite");
    if (rv == false) {
      // TODO(xairy): use auto_ptr.
      delete explosion;
      return false;
    }
    explosion->SetPosition(sf::Vector2f(snapshot->x, snapshot->y));
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
      client_options_->speed * delta_time;
    float delta_y = (keyboard_state_.down - keyboard_state_.up) *
      client_options_->speed * delta_time;

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

// FIXME(xairy): magic numbers.
// TODO(xairy): prettier HUD.
void Application::RenderHUD() {
  CHECK(state_ == STATE_INITIALIZED);

  // Sets origin to the left top corner.
  sf::Transform hud_transform;
  hud_transform.translate(view_.getCenter() - view_.getSize() / 2.0f);

  int32_t max_health = client_options_->max_health;
  float health_rect_width = 100.0f * player_health_ / max_health;
  float health_rect_height = 10.0f;
  sf::Vector2f health_rect_size(health_rect_width, health_rect_height);

  sf::RectangleShape health_rect;
  health_rect.setSize(health_rect_size);
  health_rect.setPosition(sf::Vector2f(50.0f, 510.0f));
  health_rect.setFillColor(sf::Color(0xFF, 0x00, 0xFF, 0xBB));
  render_window_->draw(health_rect, hud_transform);

  int32_t blow_capacity = client_options_->blow_capacity;
  float blow_rect_width = 100.0f * player_blow_charge_ / blow_capacity;
  float blow_rect_height = 10.0f;
  sf::Vector2f blow_rect_size(blow_rect_width, blow_rect_height);

  sf::RectangleShape blow_charge_rect;
  blow_charge_rect.setSize(blow_rect_size);
  blow_charge_rect.setPosition(sf::Vector2f(50.0f, 530.0f));
  blow_charge_rect.setFillColor(sf::Color(0x00, 0xFF, 0xFF, 0xBB));
  render_window_->draw(blow_charge_rect, hud_transform);

  int32_t morph_capacity = client_options_->morph_capacity;
  float morph_rect_width = 100.0f * player_morph_charge_ / morph_capacity;
  float morph_rect_height = 10.0f;
  sf::Vector2f morph_rect_size(morph_rect_width, morph_rect_height);

  sf::RectangleShape morph_charge_rect;
  morph_charge_rect.setSize(morph_rect_size);
  morph_charge_rect.setPosition(sf::Vector2f(50.0f, 550.0f));
  morph_charge_rect.setFillColor(sf::Color(0xFF, 0xFF, 0x00, 0xBB));
  render_window_->draw(morph_charge_rect, hud_transform);

  sf::CircleShape compass_border(60.0f);
  compass_border.setPosition(20.0f, 20.0f);  // Left top corner, not center.
  compass_border.setOutlineColor(sf::Color(0xFF, 0xFF, 0xFF, 0xFF));
  compass_border.setOutlineThickness(1.0f);
  compass_border.setFillColor(sf::Color(0xFF, 0xFF, 0xFF, 0x00));
  render_window_->draw(compass_border, hud_transform);

  int64_t render_time = GetServerTime();

  std::map<uint32_t, Object*>::const_iterator it;
  for (it = objects_.begin(); it != objects_.end(); ++it) {
    Object* obj = it->second;

    sf::Vector2f obj_pos = obj->GetPosition(render_time);
    sf::Vector2f player_pos = player_->GetPosition(render_time);
    sf::Vector2f rel = obj_pos - player_pos;
    if (Length(rel) < 400) {
      rel = rel * (60.0f / 400.0f);
      sf::CircleShape circle(1.0f);
      sf::Vector2f compass_center(80.0f, 80.0f);
      circle.setPosition(compass_center + sf::Vector2f(rel.x, rel.y));
      circle.setFillColor(sf::Color(0xFF, 0x00, 0x00, 0xFF));
      render_window_->draw(circle, hud_transform);
    }
  }

  for (it = walls_.begin(); it != walls_.end(); ++it) {
    Object* obj = it->second;

    sf::Vector2f obj_pos = obj->GetPosition(render_time);
    sf::Vector2f player_pos = player_->GetPosition(render_time);
    sf::Vector2f rel = obj_pos - player_pos;
    if (Length(rel) < 400) {
      rel = rel * (60.0f / 400.0f);
      sf::CircleShape circle(1.0f);
      sf::Vector2f compass_center(80.0f, 80.0f);
      circle.setPosition(compass_center + sf::Vector2f(rel.x, rel.y));
      circle.setFillColor(sf::Color(0x00, 0xFF, 0x00, 0xFF));
      render_window_->draw(circle, hud_transform);
    }
  }

  // Draw client.
  sf::CircleShape circle(1.0f);
  sf::Vector2f compass_center(80.0f, 80.0f);
  circle.setPosition(compass_center);
  circle.setFillColor(sf::Color(0x00, 0x00, 0xFF, 0xFF));
  render_window_->draw(circle, hud_transform);
}

}  // namespace bm
