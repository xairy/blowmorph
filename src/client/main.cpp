// Copyright (c) 2013 Blowmorph Team

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <algorithm>
#include <limits>
#include <list>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>

#include <enet-plus/enet.hpp>

#include "base/error.h"
#include "base/macros.h"
#include "base/protocol.h"
#include "base/pstdint.h"
#include "base/settings_manager.h"

#include "client/object.h"
#include "client/sprite.h"
#include "client/texture_atlas.h"

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

class Application {
 public:
  // XXX(xairy): initialize only some fields.
  Application()
      : _state(STATE_FINALIZED),
        _network_state(NETWORK_STATE_DISCONNECTED),
        _client(NULL),
        _player_morph_charge(0.0f),
        _player_blow_charge(0.0f),
        _player_health(0.0f),
        _player_size(0),
        _player(NULL),
        _render_window(NULL),
        _last_tick(0),
        _max_error(0.0f),
        _peer(NULL),
        _event(NULL),
        _is_running(false),
        _connect_timeout(0),
        _tick_rate(0),
        _wall_size(0),
        _last_loop(0),
        _client_options(NULL),
        _font(NULL),
        _time_correction(0) { }
  ~Application() {
    CHECK(_state == STATE_FINALIZED);
  }

  bool Execute() {
    if (!_settings.Open("data/client.cfg")) {
      return false;
    }

    // XXX[24.7.2012 alex]: switch to a FSM
    if (!_Initialize()) {
      return false;
    }

    _is_running = true;

    while (_is_running) {
      if (!_PumpEvents()) {
        return false;
      }
      if (!_PumpPackets(0)) {
        return false;
      }
      if (!_Loop()) {
        return false;
      }
      _Render();

      int64_t current_time = _GetTime();
      if (current_time - _last_tick > 1000.0 / _tick_rate) {
        _last_tick = current_time;
        net::SendInputEvents(_peer, _keyboard_events, _mouse_events);
      }
    }

    // _Finalize();

    return true;
  }

  void Finalize() {
    // CHECK(_state == STATE_INITIALIZED);

    if (_player != NULL) delete _player;
    if (_client_options != NULL) delete _client_options;

    if (_client != NULL) delete _client;
    if (_event != NULL) delete _event;

    std::map<int, Object*>::iterator it;
    for (it = _walls.begin(); it != _walls.end(); ++it) {
      delete it->second;
    }
    _walls.clear();

    for (it = _objects.begin(); it != _objects.end(); ++it) {
      delete it->second;
    }
    _objects.clear();

    std::list<Sprite*>::iterator it2;
    for (it2 = _explosions.begin(); it2 != _explosions.end(); ++it2) {
      delete *it2;
    }
    _explosions.clear();

    delete _font;
    delete _render_window;

    _state = STATE_FINALIZED;
  }

 private:
  bool _Initialize() {
    _is_running = false;

    _client = NULL;
    _peer = NULL;
    _event = NULL;

    _connect_timeout = 500;
    _time_correction = 0;
    _last_tick = 0;
    _tick_rate = 30;

    _player = NULL;

    _client_options = NULL;

    _wall_size = 16;
    _player_size = 30;

    // TODO(xairy): think about more accurate name.
    _max_error = 25.f;

    _player_health = 0;
    _player_blow_charge = 0;
    _player_morph_charge = 0;

    if (!_InitializeGraphics()) {
      return false;
    }

    if (!_InitializeNetwork()) {
      return false;
    }

    _last_loop = _GetTime();

    _font = new sf::Font();
    _font->loadFromFile("data/fonts/tahoma.ttf");

    _state = STATE_INITIALIZED;

    return true;
  }

  bool _InitializeGraphics() {
    uint32_t width = _settings.GetUInt32("resolution.width");
    uint32_t height = _settings.GetUInt32("resolution.height");

    sf::VideoMode video_mode(width, height);
    _render_window = new sf::RenderWindow(video_mode, "Blowmorph");
    _view.reset(sf::FloatRect(0, 0, width, height));
    _render_window->setView(_view);

    return true;
  }

  bool _InitializeNetwork() {
    if (!_enet.Initialize()) {
      return false;
    }

    std::auto_ptr<enet::ClientHost> client(_enet.CreateClientHost());
    if (client.get() == NULL) {
      return false;
    }

    std::string host = _settings.GetString("server.host");
    uint16_t port = _settings.GetUInt16("server.port");

    _peer = client->Connect(host, port);
    if (_peer == NULL) {
      return false;
    }

    std::auto_ptr<enet::Event> event(_enet.CreateEvent());
    if (event.get() == NULL) {
      return false;
    }

    CHECK(0 <= _connect_timeout);
    CHECK(_connect_timeout <= std::numeric_limits<uint32_t>::max());
    bool rv = client->Service(event.get(), (uint32_t) _connect_timeout);
    if (rv == false) {
      return false;
    }
    if (event->GetType() != enet::Event::TYPE_CONNECT) {
      BM_ERROR("Could not connect to server.");
      return false;
    }
    _client = client.release();
    _event = event.release();

    _network_state = NETWORK_STATE_CONNECTED;

    printf("Connected to %s:%u.\n", _event->GetPeer()->GetIp().c_str(),
        _event->GetPeer()->GetPort());

    return true;
  }

  bool _PumpEvents() {
    sf::Event event;
    while (_render_window->pollEvent(event)) {
      if (!_ProcessEvent(event)) {
        return false;
      }
    }
    return true;
  }

  bool _ProcessEvent(const sf::Event& event) {
    switch (event.type) {
      case sf::Event::Closed:
        return OnSFMLQuit(event);
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

  bool OnMouseButtonEvent(const sf::Event& event) {
    MouseEvent mouse_event;
    mouse_event.time = _GetTime();

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

    int screenWidth = _render_window->getSize().x;
    int screenHeight = _render_window->getSize().y;
    mouse_event.x = (event.mouseButton.x - screenWidth / 2) +
      _player->GetPosition().x;
    mouse_event.y = (event.mouseButton.y - screenHeight / 2) +
      _player->GetPosition().y;
    _mouse_events.push_back(mouse_event);

    return true;
  }

  bool OnSFMLQuit(const sf::Event& event) {
    _is_running = false;

    CHECK(0 <= _connect_timeout);
    CHECK(_connect_timeout <= std::numeric_limits<uint32_t>::max());

    if (!net::DisconnectPeer(_peer, _event, _client, _connect_timeout)) {
      BM_ERROR("Did not receive EVENT_DISCONNECT event while disconnecting.\n");
      return false;
    } else {
      printf("Client disconnected.\n");
    }

    return true;
  }

  bool OnKeyEvent(const sf::Event& event) {
    KeyboardEvent keyboard_event;
    keyboard_event.time = _GetTime();

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
        _keyboard_state.left = pressed;
        keyboard_event.key_type = KeyboardEvent::KEY_LEFT;
        break;
      case sf::Keyboard::D:
        _keyboard_state.right = pressed;
        keyboard_event.key_type = KeyboardEvent::KEY_RIGHT;
        break;
      case sf::Keyboard::W:
        _keyboard_state.up = pressed;
        keyboard_event.key_type = KeyboardEvent::KEY_UP;
        break;
      case sf::Keyboard::S:
        _keyboard_state.down = pressed;
        keyboard_event.key_type = KeyboardEvent::KEY_DOWN;
        break;
      case sf::Keyboard::Escape: {
        _is_running = !pressed;

        CHECK(0 <= _connect_timeout);
        CHECK(_connect_timeout <= std::numeric_limits<uint32_t>::max());
        if (!net::DisconnectPeer(_peer, _event, _client, _connect_timeout)) {
          BM_ERROR("Did not receive EVENT_DISCONNECT while disconnecting.\n");
          return false;
        } else {
          printf("Client disconnected.\n");
        }
      } break;
      default:
        break;
    }

    _keyboard_events.push_back(keyboard_event);

    return true;
  }

  bool _PumpPackets(uint32_t timeout) {
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

      uint32_t service_timeout = timeout == 0 ?
          0 : (timeout - (time - start_time));
      bool rv = _client->Service(_event, service_timeout);
      if (rv == false) {
        return false;
      }

      switch (_event->GetType()) {
        case enet::Event::TYPE_RECEIVE: {
          _event->GetData(&message);

          const Packet::Type* type =
              reinterpret_cast<Packet::Type*>(&message[0]);
          const void* data =
              reinterpret_cast<void*>(&message[0] + sizeof(Packet::Type));
          size_t len = message.size() - sizeof(Packet::Type);

          // printf("Received %d.", *type);

          // XXX[24.7.2012 alex]: will only process single packet at a time
          ProcessPacket(*type, data, len);
        } break;

        case enet::Event::TYPE_CONNECT: {
          sys::Warning("Got EVENT_CONNECT while being already connected.");
        } break;

        case enet::Event::TYPE_DISCONNECT: {
          _network_state = NETWORK_STATE_DISCONNECTED;
          _is_running = false;
          BM_ERROR("Connection lost.");
          return false;
        } break;

        case enet::Event::TYPE_NONE:
          break;
      }
    } while (_event->GetType() != enet::Event::TYPE_NONE);

    return true;
  }

  void DeleteObject(int id) {
    typedef std::map<int, Object*>::iterator It;
    It it = _objects.find(id);
    if (it != _objects.end()) {
      delete it->second;
      _objects.erase(it);
    }

    it = _walls.find(id);
    if (it != _walls.end()) {
      delete it->second;
      _walls.erase(it);
    }
  }

  bool ProcessPacket(Packet::Type type, const void* data, size_t len) {
    switch (_network_state) {
      case NETWORK_STATE_DISCONNECTED: {
        sys::Warning("Received a packet while being in disconnected state.");
        return true;
      } break;
      case NETWORK_STATE_CONNECTED: {
        if (type != Packet::TYPE_CLIENT_OPTIONS) {
          sys::Warning("Received packet with a type %d "
              "while waiting for client options.", type);
        } else if (len != sizeof(ClientOptions)) {
          sys::Warning("Received packet has incorrect length.");
        } else {
          const ClientOptions* options =
              reinterpret_cast<const ClientOptions*>(data);
          _client_options = new ClientOptions(*options);

          // Switch to a new state.
          _network_state = NETWORK_STATE_SYNCHRONIZATION;

          // Send a time synchronization request.
          TimeSyncData request_data;
          request_data.client_time = sys::Timestamp();

          std::vector<char> buf;
          net::AppendPacketToBuffer(buf, &request_data,
              Packet::TYPE_SYNC_TIME_REQUEST);

          bool rv = _peer->Send(&buf[0], buf.size(), true);
          if (rv == false) {
            return false;
          }
        }
        return true;
      } break;
      case NETWORK_STATE_SYNCHRONIZATION: {
        if (type != Packet::TYPE_SYNC_TIME_RESPONSE) {
          sys::Warning("Received packet with a type %d "
              "while waiting for time sync response.", type);
        } else if (len != sizeof(TimeSyncData)) {
          sys::Warning("Received packet has incorrect length.");
        } else {
          const TimeSyncData* response_data =
              reinterpret_cast<const TimeSyncData*>(data);

          // Calculate the time correction.
          int64_t client_time = sys::Timestamp();
          int64_t latency = (client_time - response_data->client_time) / 2;
          _time_correction = response_data->server_time + latency - client_time;

          // XXX(xairy): linux x64: uint64_t == long int == %ld != %lld.
          // printf("Time correction is %lld ms.\n", _time_correction);
          // printf("Latency is %lld.\n", latency);

          _network_state = NETWORK_STATE_LOGGED_IN;

          // XXX(alex): move it to the ProcessPacket method
          sf::Vector2f player_pos(_client_options->x, _client_options->y);
          _player = new Object(player_pos, 0, _client_options->id,
              EntitySnapshot::ENTITY_TYPE_PLAYER, "data/sprites/mechos.sprite");
          CHECK(_player != NULL);
          // XXX(alex): maybe we should have a xml file for each object with
          //            texture paths, pivots, captions, etc
          _player->SetPosition(player_pos);
          _player->visible = true;
          _player->name_visible = true;
        }

        return true;
      } break;
      case NETWORK_STATE_LOGGED_IN: {
        bool isSnapshot = len == sizeof(EntitySnapshot) &&
                          (type == Packet::TYPE_ENTITY_APPEARED ||
                           type == Packet::TYPE_ENTITY_DISAPPEARED ||
                           type == Packet::TYPE_ENTITY_UPDATED);

        if (isSnapshot) {
          const EntitySnapshot* snapshot =
              reinterpret_cast<const EntitySnapshot*>(data);
          if (type == Packet::TYPE_ENTITY_DISAPPEARED) {
            if (!OnEntityDisappearance(snapshot)) {
              return false;
            }
          } else {
            if (snapshot->id == _player->id) {
              OnPlayerUpdate(snapshot);
              break;
            }

            if (_objects.count(snapshot->id) > 0 ||
                _walls.count(snapshot->id) > 0) {
              OnEntityUpdate(snapshot);
            } else {
              OnEntityAppearance(snapshot);
            }
          }
        } else {
          sys::Warning("Received packet is not an entity snapshot.");
        }

        return true;
      } break;

      default:
        return false;
        break;
    }
    return false;
  }

  void OnEntityAppearance(const EntitySnapshot* snapshot) {
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
        _walls[snapshot->id] = new Object(position, time, snapshot->id,
            snapshot->type, "data/sprites/wall.sprite");
        _walls[snapshot->id]->sprite.SetCurrentFrame(tile);
        _walls[snapshot->id]->EnableInterpolation();
        _walls[snapshot->id]->visible = true;
        _walls[snapshot->id]->name_visible = false;
      } break;

      case EntitySnapshot::ENTITY_TYPE_BULLET: {
        _objects[snapshot->id] = new Object(position, time, snapshot->id,
            snapshot->type, "data/sprites/bullet.sprite");
        _objects[snapshot->id]->EnableInterpolation();
        _objects[snapshot->id]->visible = true;
        _objects[snapshot->id]->name_visible = false;
      } break;

      case EntitySnapshot::ENTITY_TYPE_PLAYER: {
        _objects[snapshot->id] = new Object(position, time, snapshot->id,
            snapshot->type, "data/sprites/mechos.sprite");
        _objects[snapshot->id]->EnableInterpolation();
        _objects[snapshot->id]->visible = true;
        _objects[snapshot->id]->name_visible = true;
      } break;

      case EntitySnapshot::ENTITY_TYPE_DUMMY: {
        _objects[snapshot->id] = new Object(position, time, snapshot->id,
            snapshot->type, "data/sprites/dummy.sprite");
        _objects[snapshot->id]->EnableInterpolation();
        _objects[snapshot->id]->visible = true;
        _objects[snapshot->id]->name_visible = false;
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
        _objects[snapshot->id] = new Object(position, time, snapshot->id,
            snapshot->type, "data/sprites/station.sprite");
        _objects[snapshot->id]->sprite.SetCurrentFrame(tile);
        _objects[snapshot->id]->EnableInterpolation();
        _objects[snapshot->id]->visible = true;
        _objects[snapshot->id]->name_visible = false;
      } break;

      default:
        break;
    }
  }

  void OnEntityUpdate(const EntitySnapshot* snapshot) {
    CHECK(snapshot != NULL);

    int64_t time = snapshot->time;
    sf::Vector2f position = sf::Vector2f(snapshot->x, snapshot->y);

    ObjectState state;
    state.position = position;
    state.health = static_cast<float>(snapshot->data[0]);
    state.blowCharge = static_cast<float>(snapshot->data[1]);
    state.morphCharge = static_cast<float>(snapshot->data[2]);

    if (snapshot->type == EntitySnapshot::ENTITY_TYPE_WALL) {
      _walls[snapshot->id]->UpdateState(state, time);
    } else {
      _objects[snapshot->id]->UpdateState(state, time);
    }
  }

  void OnPlayerUpdate(const EntitySnapshot* snapshot) {
    CHECK(snapshot != NULL);

    sf::Vector2f position = sf::Vector2f(snapshot->x, snapshot->y);
    sf::Vector2f distance = _player->GetPosition() - position;

    ObjectState state;
    state.position = position;
    _player_health = static_cast<float>(snapshot->data[0]);
    _player_blow_charge = static_cast<float>(snapshot->data[1]);
    _player_morph_charge = static_cast<float>(snapshot->data[2]);

    if (Length(distance) > _max_error) {
      _player->EnforceState(state, snapshot->time);
    } else {
      // _player->UpdateCurrentState(state, snapshot->time);
    }
  }

  bool OnEntityDisappearance(const EntitySnapshot* snapshot) {
    CHECK(snapshot != NULL);

    DeleteObject(snapshot->id);

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
      _explosions.push_back(explosion);
    }

    return true;
  }

  bool _Loop() {
    if (_network_state == NETWORK_STATE_LOGGED_IN) {
      int64_t current_time = _GetTime();
      int64_t delta_time = current_time - _last_loop;
      _last_loop = current_time;

      float delta_x = (_keyboard_state.right - _keyboard_state.left) *
        _client_options->speed * delta_time;
      float delta_y = (_keyboard_state.down - _keyboard_state.up) *
        _client_options->speed * delta_time;

      bool intersection_x = false;
      bool intersection_y = false;

      std::map<int, Object*>::iterator it;
      sf::Vector2f player_pos = _player->GetPosition(current_time);

      for (it = _walls.begin() ; it != _walls.end(); it++) {
        sf::Vector2f wall_pos = it->second->GetPosition(current_time);
        float player_to_wall_x = abs(wall_pos.x - (player_pos.x + delta_x));
        float player_to_wall_y = abs(wall_pos.y - player_pos.y);
        if (player_to_wall_x < (_player_size + _wall_size) / 2 &&
            player_to_wall_y < (_player_size + _wall_size) / 2) {
          intersection_x = true;
          break;
        }
      }
      for (it = _walls.begin() ; it != _walls.end(); it++) {
        sf::Vector2f wall_pos = it->second->GetPosition(current_time);
        float player_to_wall_x = abs(wall_pos.x - player_pos.x);
        float player_to_wall_y = abs(wall_pos.y - (player_pos.y + delta_y));
        if (player_to_wall_x < (_player_size + _wall_size) / 2 &&
            player_to_wall_y < (_player_size + _wall_size) / 2) {
          intersection_y = true;
          break;
        }
      }

      if (!intersection_x) _player->Move(sf::Vector2f(delta_x, 0.0f));
      if (!intersection_y) _player->Move(sf::Vector2f(0.0f, delta_y));
    }

    return true;
  }

  // Returns approximate server time (with the correction).
  int64_t _GetTime() {
    return sys::Timestamp() + _time_correction;
  }

  // FIXME(xairy): magic numbers.
  // TODO(xairy): prettier HUD.
  void _RenderHUD() {
    // Sets origin to the left top corner.
    sf::Transform hud_transform;
    hud_transform.translate(_view.getCenter() - _view.getSize() / 2.0f);

    int32_t max_health = _client_options->max_health;
    float health_rect_width = 100.0f * _player_health / max_health;
    float health_rect_height = 10.0f;
    sf::Vector2f health_rect_size(health_rect_width, health_rect_height);

    sf::RectangleShape health_rect;
    health_rect.setSize(health_rect_size);
    health_rect.setPosition(sf::Vector2f(50.0f, 510.0f));
    health_rect.setFillColor(sf::Color(0xFF, 0x00, 0xFF, 0xBB));
    _render_window->draw(health_rect, hud_transform);

    int32_t blow_capacity = _client_options->blow_capacity;
    float blow_rect_width = 100.0f * _player_blow_charge / blow_capacity;
    float blow_rect_height = 10.0f;
    sf::Vector2f blow_rect_size(blow_rect_width, blow_rect_height);

    sf::RectangleShape blow_charge_rect;
    blow_charge_rect.setSize(blow_rect_size);
    blow_charge_rect.setPosition(sf::Vector2f(50.0f, 530.0f));
    blow_charge_rect.setFillColor(sf::Color(0x00, 0xFF, 0xFF, 0xBB));
    _render_window->draw(blow_charge_rect, hud_transform);

    int32_t morph_capacity = _client_options->morph_capacity;
    float morph_rect_width = 100.0f * _player_morph_charge / morph_capacity;
    float morph_rect_height = 10.0f;
    sf::Vector2f morph_rect_size(morph_rect_width, morph_rect_height);

    sf::RectangleShape morph_charge_rect;
    morph_charge_rect.setSize(morph_rect_size);
    morph_charge_rect.setPosition(sf::Vector2f(50.0f, 550.0f));
    morph_charge_rect.setFillColor(sf::Color(0xFF, 0xFF, 0x00, 0xBB));
    _render_window->draw(morph_charge_rect, hud_transform);

    sf::CircleShape compass_border(60.0f);
    compass_border.setPosition(20.0f, 20.0f);  // Left top corner, not center.
    compass_border.setOutlineColor(sf::Color(0xFF, 0xFF, 0xFF, 0xFF));
    compass_border.setOutlineThickness(1.0f);
    compass_border.setFillColor(sf::Color(0xFF, 0xFF, 0xFF, 0x00));
    _render_window->draw(compass_border, hud_transform);

    int64_t render_time = _GetTime();

    std::map<int, Object*>::const_iterator it;
    for (it = _objects.begin(); it != _objects.end(); ++it) {
      Object* obj = it->second;

      sf::Vector2f obj_pos = obj->GetPosition(render_time);
      sf::Vector2f player_pos = _player->GetPosition(render_time);
      sf::Vector2f rel = obj_pos - player_pos;
      if (Length(rel) < 400) {
        rel = rel * (60.0f / 400.0f);
        sf::CircleShape circle(1.0f);
        sf::Vector2f compass_center(80.0f, 80.0f);
        circle.setPosition(compass_center + sf::Vector2f(rel.x, rel.y));
        circle.setFillColor(sf::Color(0xFF, 0x00, 0x00, 0xFF));
        _render_window->draw(circle, hud_transform);
      }
    }

    for (it = _walls.begin(); it != _walls.end(); ++it) {
      Object* obj = it->second;

      sf::Vector2f obj_pos = obj->GetPosition(render_time);
      sf::Vector2f player_pos = _player->GetPosition(render_time);
      sf::Vector2f rel = obj_pos - player_pos;
      if (Length(rel) < 400) {
        rel = rel * (60.0f / 400.0f);
        sf::CircleShape circle(1.0f);
        sf::Vector2f compass_center(80.0f, 80.0f);
        circle.setPosition(compass_center + sf::Vector2f(rel.x, rel.y));
        circle.setFillColor(sf::Color(0x00, 0xFF, 0x00, 0xFF));
        _render_window->draw(circle, hud_transform);
      }
    }

    {
      Object* obj = _player;

      sf::Vector2f obj_pos = obj->GetPosition(render_time);
      sf::Vector2f player_pos = _player->GetPosition(render_time);
      sf::Vector2f rel = obj_pos - player_pos;
      if (Length(rel) < 400) {
        rel = rel * (60.0f / 400.0f);
        sf::CircleShape circle(1.0f);
        sf::Vector2f compass_center(80.0f, 80.0f);
        circle.setPosition(compass_center + sf::Vector2f(rel.x, rel.y));
        circle.setFillColor(sf::Color(0x00, 0x00, 0xFF, 0xFF));
        _render_window->draw(circle, hud_transform);
      }
    }
  }

  // Renders everything.
  void _Render() {
    _render_window->clear();

    if (_network_state == NETWORK_STATE_LOGGED_IN) {
      int64_t render_time = _GetTime();
      sf::Vector2f player_pos = _player->GetPosition(render_time);

      _view.setCenter(Round(player_pos));
      _render_window->setView(_view);

      std::list<Sprite*>::iterator it2;
      for (it2 = _explosions.begin(); it2 != _explosions.end();) {
        (*it2)->Render(_render_window);
        if ((*it2)->IsStopped()) {
          std::list<Sprite*>::iterator it1 = it2;
          ++it1;
          delete *it2;
          _explosions.erase(it2);
          it2 = it1;
        } else {
          ++it2;
        }
      }

      std::map<int, Object*>::iterator it;
      for (it = _walls.begin() ; it != _walls.end(); ++it) {
        RenderObject(it->second, render_time, _font, *_render_window);
      }
      for (it = _objects.begin() ; it != _objects.end(); ++it) {
        RenderObject(it->second, render_time, _font, *_render_window);
      }

      RenderObject(_player, render_time, _font, *_render_window);

      _RenderHUD();
    }

    _render_window->display();
  }

  bool _is_running;

  sf::Font* _font;

  enet::Enet _enet;
  enet::ClientHost* _client;
  enet::Peer* _peer;
  enet::Event* _event;

  // In milliseconds.
  int64_t _connect_timeout;
  int64_t _time_correction;
  int64_t _last_tick;
  int _tick_rate;
  int64_t _last_loop;

  sf::RenderWindow* _render_window;
  sf::View _view;

  Object* _player;

  std::map<int, Object*> _objects;
  std::map<int, Object*> _walls;

  std::list<Sprite*> _explosions;

  ClientOptions* _client_options;

  uint32_t _wall_size;
  uint32_t _player_size;

  float _max_error;

  float _player_health;
  float _player_blow_charge;
  float _player_morph_charge;

  // Input events since the last tick.
  std::vector<KeyboardEvent> _keyboard_events;
  std::vector<MouseEvent> _mouse_events;

  struct KeyboardState {
    KeyboardState() : up(false), down(false), right(false), left(false) { }

    bool up;
    bool down;
    bool right;
    bool left;
  };
  KeyboardState _keyboard_state;

  enum State {
    STATE_INITIALIZED,
    STATE_FINALIZED
  };
  State _state;

  enum NetworkState {
    NETWORK_STATE_DISCONNECTED,
    NETWORK_STATE_CONNECTED,
    NETWORK_STATE_SYNCHRONIZATION,
    NETWORK_STATE_LOGGED_IN
  };
  NetworkState _network_state;

  SettingsManager _settings;
};

}  // namespace bm

int main(int argc, char** argv) {
  bm::Application app;
  if (!app.Execute()) {
    bm::Error::Print();
    app.Finalize();
    return EXIT_FAILURE;
  }
  app.Finalize();
  return EXIT_SUCCESS;
}
