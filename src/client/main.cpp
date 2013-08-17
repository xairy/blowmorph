#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <algorithm>
#include <iostream>
#include <limits>
#include <list>
#include <map>
#include <memory>
#include <string>

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <glm/glm.hpp>
#include <FreeImage.h>

#include <base/error.hpp>
#include <base/macros.hpp>
#include <base/protocol.hpp>
#include <base/pstdint.hpp>

#include <enet-plus/enet.hpp>
#include <interpolator/interpolator.hpp>
#include <ini-file/ini_file.hpp>

#include "engine/animation.hpp"
#include "engine/sprite.hpp"
#include "engine/texture_atlas.hpp"

#include "sys.hpp"
#include "net.hpp"

bm::TimeType getTicks() {
  return sys::Timestamp();
}

struct ObjectState {
  glm::vec2 position;
  float blowCharge;
  float morphCharge;
  float health;
};

namespace interpolator {
  template<> glm::vec2 lerp(const glm::vec2& a, const glm::vec2& b, double bRatio) {
    glm::vec2 result;
    result.x = lerp(a.x, b.x, bRatio);
    result.y = lerp(a.y, b.y, bRatio);
    return result;
  }
  template<> ObjectState lerp(const ObjectState& a, const ObjectState& b, double bRatio) {
    ObjectState result;
    result.position = lerp(a.position, b.position, bRatio);
    result.blowCharge = lerp(a.blowCharge, b.blowCharge, bRatio);
    result.morphCharge = lerp(a.morphCharge, b.morphCharge, bRatio);
    result.health = lerp(a.health, b.health, bRatio);
    return result;
  }
};

typedef interpolator::LinearInterpolator<ObjectState, bm::TimeType> ObjectInterpolator;

// TODO[24.7.2012 alex]: fix method names
struct Object {
  // FIXME[18.11.2012 alex]: hardcoded initial interpolation time step.
  Object(const glm::vec2& position, bm::TimeType time, uint32_t id, uint32_t type, bm::TextureAtlas* texture, size_t tile = 0)
      : id(id),
        type(type),
        visible(false),
        interpolation_enabled(false),
        name_visible(false),
        interpolator(ObjectInterpolator(bm::TimeType(75), 1)),
        sprite(texture, tile) {
    ObjectState state;
    state.blowCharge = 0;
    state.health = 0;
    state.morphCharge = 0;
    state.position = position;
    interpolator.Push(state, time);

    name_render_offset = glm::vec2(-15.0f, -30.0f);
  }

  void EnableInterpolation() {
    interpolation_enabled = true;
    interpolator.SetFrameCount(2);
  }

  void DisableInterpolation() {
    interpolation_enabled = false;
    interpolator.SetFrameCount(1);
  }

  void EnforceState(const ObjectState& state, bm::TimeType time) {
    interpolator.Clear();
    interpolator.Push(state, time);
  }

  void UpdateCurrentState(const ObjectState& state, bm::TimeType time) {
    interpolator.Push(state, time);
  }

  glm::vec2 GetPosition(bm::TimeType time) {
    return interpolator.Interpolate(time).position;
  }

  glm::vec2 GetPosition() {
    assert(!interpolation_enabled);
    return GetPosition(bm::TimeType(0));
  }

  void SetPosition(const glm::vec2& value) {
    assert(!interpolation_enabled);
    ObjectState state = interpolator.Interpolate(bm::TimeType(0));
    state.position = value;
    EnforceState(state, bm::TimeType(0));
  }

  void Move(const glm::vec2& value) {
    assert(!interpolation_enabled);
    ObjectState state = interpolator.Interpolate(bm::TimeType(0));
    state.position = state.position + value;
    EnforceState(state, bm::TimeType(0));
  }

  uint32_t id;
  uint32_t type;

  bm::Sprite sprite;

  bool visible;

  bool name_visible;
  glm::vec2 name_render_offset;

  bool interpolation_enabled;
  ObjectInterpolator interpolator;
};

void RenderObject(Object* object, bm::TimeType time, std::map<uint32_t, bm::TextureAtlas*> textures,
                  sf::Font* font, sf::RenderWindow& render_window) {
  CHECK(object != NULL);
  CHECK(font != NULL);

  ObjectState state = object->interpolator.Interpolate(time);

  if (object->visible) {
    object->sprite.SetPosition(glm::round(state.position));
    object->sprite.Render(render_window);
  }

  if (object->name_visible) {
    glm::vec2 caption_position = glm::round(state.position + object->name_render_offset);
    sf::Text text("Myself", *font, 12);
    text.setPosition(caption_position.x, caption_position.y);
    render_window.draw(text);
  }
}

using namespace bm;

class Application {
public:
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
        default_text_writer(NULL),
        _time_correction(0) { }
  ~Application() {
    CHECK(_state == STATE_FINALIZED);
  }

  bool Execute() {
    if (!IniFile::LoadINI("data/client.ini", settings)) {
      return false;
    }

    // XXX[24.7.2012 alex]: switch to a FSM
    if(!_Initialize()) {
      return false;
    }

    _is_running = true;

    while(_is_running) {
      if(!_PumpEvents()) {
        return false;
      }
      if(!_PumpPackets(0)) {
        return false;
      }
      if(!_Loop()) {
        return false;
      }
      _Render();

      TimeType current_time = _GetTime();
      if(current_time - _last_tick > 1000.0 / _tick_rate) {
        _last_tick = current_time;
        bm::net::SendInputEvents(_peer, _keyboard_events, _mouse_events);
      }
    }

    //_Finalize();

    return true;
  }

  void Finalize() {
    //CHECK(_state == STATE_INITIALIZED);

    // Delete all loaded textures.
    for (std::map<uint32_t, TextureAtlas*>::iterator it = textures.begin(); it != textures.end(); it++) {
      delete it->second;
    }
    textures.clear();

    if(_player != NULL) delete _player;
    if(_client_options != NULL) delete _client_options;

    if(_client != NULL) delete _client;
    if(_event != NULL) delete _event;

    std::map<int, Object*>::iterator it;
    for(it = _walls.begin(); it != _walls.end(); ++it) {
      delete it->second;
    }
    _walls.clear();

    for(it = _objects.begin(); it != _objects.end(); ++it) {
      delete it->second;
    }
    _objects.clear();

    std::list<Animation*>::iterator it2;
    for(it2 = _animations.begin(); it2 != _animations.end(); ++it2) {
      delete *it2;
    }
    _animations.clear();

    delete default_text_writer;

    delete _render_window;

    _state = STATE_FINALIZED;
  }

private:
  bool _Initialize() {
    _is_running = false;

    _client = NULL;
    _peer = NULL;
    _event = NULL;

    _connect_timeout = TimeType(500);
    _time_correction = TimeType(0);
    _last_tick = TimeType(0);
    _tick_rate = 30;

    _player = NULL;

    _client_options = NULL;

    _wall_size = 16;
    _player_size = 30;

    // TODO: think about more accurate name.
    _max_error = 25.f;

    _player_health = 0;
    _player_blow_charge = 0;
    _player_morph_charge = 0;

    if(!_InitializeGraphics()) {
      return false;
    }

    if(!_InitializeNetwork()) {
      return false;
    }

    _last_loop = _GetTime();

    default_text_writer = new sf::Font();
    default_text_writer->loadFromFile("data/fonts/tahoma.ttf");

    _state = STATE_INITIALIZED;

    return true;
  }

  bool _InitializeGraphics() {
    _render_window = new sf::RenderWindow(sf::VideoMode(600, 600), "Blowmorph");
    _view.reset(sf::FloatRect(0, 0, 600, 600));
    _render_window->setView(_view);

    // Load all textures.
    textures[EntitySnapshot::ENTITY_TYPE_PLAYER] = bm::LoadOldTexture("data/images/player.png", 0);
    textures[EntitySnapshot::ENTITY_TYPE_BULLET] = bm::LoadOldTexture("data/images/bullet.png", (8 << 16) + (54 << 8) + 129);
    textures[EntitySnapshot::ENTITY_TYPE_WALL] = bm::LoadTileset("data/images/walls.png", 0, 1, 1, 17, 17, 16, 16);
    textures[EntitySnapshot::ENTITY_TYPE_DUMMY] = bm::LoadOldTexture("data/images/guy.png", 0);
    textures[EntitySnapshot::ENTITY_TYPE_EXPLOSION] = bm::LoadTileset("data/images/explosion.png", 0, 1, 1, 61, 61, 60, 60);
    textures[EntitySnapshot::ENTITY_TYPE_STATION] = bm::LoadTileset("data/images/kits.png", 0, 1, 1, 31, 31, 30, 30);

    // Check if all the textures were loaded successfully.
    for (std::map<uint32_t, TextureAtlas*>::iterator it = textures.begin(); it != textures.end(); it++) {
      if (it->second == NULL) {
        return false;
      }
    }

    return true;
  }

  bool _InitializeNetwork() {
    if(!_enet.Initialize()) {
      return false;
    }

    std::auto_ptr<enet::ClientHost> client(_enet.CreateClientHost());
    if(client.get() == NULL) {
      return false;
    }

    std::string host = IniFile::GetValue<std::string>(settings, "server.host", "127.0.0.1");
    uint16_t port = IniFile::GetValue(settings, "server.port", 4242);

    std::auto_ptr<enet::Peer> peer(client->Connect(host, port));
    if(peer.get() == NULL) {
      return false;
    }

    std::auto_ptr<enet::Event> event(_enet.CreateEvent());
    if(event.get() == NULL) {
      return false;
    }

    CHECK(0 <= _connect_timeout && _connect_timeout <= std::numeric_limits<uint32_t>::max());
    bool rv = client->Service(event.get(), (uint32_t) _connect_timeout);
    if(rv == false) {
      return false;
    }
    if(event->GetType() != enet::Event::TYPE_CONNECT) {
      BM_ERROR("Could not connect to server.");
      return false;
    }
    _client = client.release();
    _peer = peer.release();
    _event = event.release();

    _network_state = NETWORK_STATE_CONNECTED;

    printf("Connected to %s:%u.\n", _event->GetPeer()->GetIp().c_str(), _event->GetPeer()->GetPort());

    return true;
  }

  bool _PumpEvents() {
    sf::Event event;
    while(_render_window->pollEvent(event)) {
      if(!_ProcessEvent(event)) {
        return false;
      }
    }
    return true;
  }

  bool _ProcessEvent(const sf::Event& event) {
    switch(event.type) {
      case sf::Event::Closed:
        return OnSDLQuit(event);
      case sf::Event::KeyPressed:
      case sf::Event::KeyReleased:
        return OnKeyEvent(event);
      case sf::Event::MouseMoved:
        break;
      case sf::Event::MouseButtonPressed:
      case sf::Event::MouseButtonReleased:
        return OnMouseButtonEvent(event);
      default: break;
    }

    return true;
  }

  bool OnMouseButtonEvent(const sf::Event& event) {
    MouseEvent mouse_event;
    mouse_event.time = _GetTime();

    if(event.mouseButton.button == sf::Mouse::Left) {
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
    mouse_event.x = (event.mouseButton.x - screenWidth / 2) + _player->GetPosition().x;
    mouse_event.y = (event.mouseButton.y - screenHeight / 2) + _player->GetPosition().y;
    _mouse_events.push_back(mouse_event);

    return true;
  }

  bool OnSDLQuit(const sf::Event& event) {
    _is_running = false;

    CHECK(0 <= _connect_timeout && _connect_timeout <= std::numeric_limits<uint32_t>::max());
    if (!bm::net::DisconnectPeer(_peer, _event, _client, (uint32_t) _connect_timeout)) {
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

    switch(event.key.code) {
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

        // TODO[xairy]: use limits.
        CHECK(0 <= _connect_timeout && _connect_timeout <= std::numeric_limits<uint32_t>::max());
        if (!bm::net::DisconnectPeer(_peer, _event, _client, (uint32_t) _connect_timeout)) {
          BM_ERROR("Did not receive EVENT_DISCONNECT event while disconnecting.\n");
          return false;
        } else {
          printf("Client disconnected.\n");
        }

        break;
      }
    }

    _keyboard_events.push_back(keyboard_event);

    return true;
  }

  bool _PumpPackets(uint32_t timeout) {
    std::vector<char> message;

    TimeType start_time = getTicks();
    do {
      TimeType time = getTicks();
      CHECK(time >= start_time);

      // If we have run out of time, break and return
      // unless timeout is zero.
      if (time - start_time > timeout && timeout != 0) {
        break;
      }
      uint32_t service_timeout = timeout == 0 ? 0 : timeout - (time - start_time);

      bool rv = _client->Service(_event, service_timeout);
      if(rv == false) {
        return false;
      }
      switch(_event->GetType()) {
        case enet::Event::TYPE_RECEIVE: {
          _event->GetData(&message);

          const Packet::Type* type = reinterpret_cast<Packet::Type*>(&message[0]);
          const void* data = reinterpret_cast<void*>(&message[0] + sizeof(Packet::Type));
          size_t len = message.size() - sizeof(Packet::Type);

          //printf("Received %d.", *type);

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
          sys::Warning("Received packet with a type %d while waiting for client options.", type);
        } else if (len != sizeof(ClientOptions)) {
          sys::Warning("Received packet has incorrect length.");
        } else {
          const ClientOptions* options = reinterpret_cast<const ClientOptions*>(data);
          _client_options = new ClientOptions(*options);

          // Switch to a new state.
          _network_state = NETWORK_STATE_SYNCHRONIZATION;

          // Send a time synchronization request.
          TimeSyncData request_data;
          request_data.client_time = getTicks();

          std::vector<char> buf;
          bm::net::AppendPacketToBuffer(buf, &request_data, Packet::TYPE_SYNC_TIME_REQUEST);

          bool rv = _peer->Send(&buf[0], buf.size(), true);
          if(rv == false) {
            return false;
          }
        }
        return true;
      } break;
      case NETWORK_STATE_SYNCHRONIZATION: {
        if (type != Packet::TYPE_SYNC_TIME_RESPONSE) {
          sys::Warning("Received packet with a type %d while waiting for time sync response.", type);
        } else if (len != sizeof(TimeSyncData)) {
          sys::Warning("Received packet has incorrect length.");
        } else {
          const TimeSyncData* response_data = reinterpret_cast<const TimeSyncData*>(data);
          // Calculate the time correction.
          TimeType client_time = getTicks();
          TimeType latency = (client_time - response_data->client_time) / 2;
          _time_correction = response_data->server_time + latency - client_time;

          // XXX[xairy]: linux x64: uint64_t == long int == %ld != %lld.
          //printf("Time correction is %lld ms.\n", _time_correction);
          //printf("Latency is %lld.\n", latency);

          _network_state = NETWORK_STATE_LOGGED_IN;

          // XXX[24.7.2012 alex]: move it to the ProcessPacket method
          _player = new Object(glm::vec2(_client_options->x, _client_options->y), TimeType(0),
                               _client_options->id, EntitySnapshot::ENTITY_TYPE_PLAYER, textures[EntitySnapshot::ENTITY_TYPE_PLAYER]);
          CHECK(_player != NULL);
          // XXX[24.7.2012 alex]: maybe we should have a xml file for each object with
          //                      texture paths, pivots, captions, etc
          _player->SetPosition(glm::vec2(_client_options->x, _client_options->y));
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
          const EntitySnapshot* snapshot = reinterpret_cast<const EntitySnapshot*>(data);
          if (type == Packet::TYPE_ENTITY_DISAPPEARED) {
            if (!OnEntityDisappearance(snapshot)) {
              return false;
            }
          } else {
            if(snapshot->id == _player->id) {
              OnPlayerUpdate(snapshot);
              break;
            }

            if(_objects.count(snapshot->id) > 0 || _walls.count(snapshot->id) > 0) {
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

    TimeType time = snapshot->time;
    glm::vec2 position = glm::vec2(snapshot->x, snapshot->y);

    switch(snapshot->type) {
      case EntitySnapshot::ENTITY_TYPE_WALL: {
        size_t tile;
        if(snapshot->data[0] == EntitySnapshot::WALL_TYPE_ORDINARY) {
          tile = 3;
        } else if(snapshot->data[0] == EntitySnapshot::WALL_TYPE_UNBREAKABLE) {
          tile = 2;
        } else if(snapshot->data[0] == EntitySnapshot::WALL_TYPE_MORPHED) {
          tile = 1;
        }
        _walls[snapshot->id] = new Object(position, time, snapshot->id, snapshot->type, textures[EntitySnapshot::ENTITY_TYPE_WALL], tile);
        _walls[snapshot->id]->EnableInterpolation();
        _walls[snapshot->id]->visible = true;
        _walls[snapshot->id]->name_visible = false;
      } break;

      case EntitySnapshot::ENTITY_TYPE_BULLET: {
        _objects[snapshot->id] = new Object(position, time, snapshot->id, snapshot->type, textures[EntitySnapshot::ENTITY_TYPE_BULLET]);
        _objects[snapshot->id]->EnableInterpolation();
        _objects[snapshot->id]->visible = true;
        _objects[snapshot->id]->name_visible = false;
      } break;

      case EntitySnapshot::ENTITY_TYPE_PLAYER: {
        _objects[snapshot->id] = new Object(position, time, snapshot->id, snapshot->type, textures[EntitySnapshot::ENTITY_TYPE_PLAYER]);
        _objects[snapshot->id]->EnableInterpolation();
        _objects[snapshot->id]->visible = true;
        _objects[snapshot->id]->name_visible = true;
      } break;

      case EntitySnapshot::ENTITY_TYPE_DUMMY: {
        _objects[snapshot->id] = new Object(position, time, snapshot->id, snapshot->type, textures[EntitySnapshot::ENTITY_TYPE_DUMMY]);
        _objects[snapshot->id]->EnableInterpolation();
        _objects[snapshot->id]->visible = true;
        _objects[snapshot->id]->name_visible = false;
      } break;

      case EntitySnapshot::ENTITY_TYPE_STATION: {
        size_t tile;
        if(snapshot->data[0] == EntitySnapshot::STATION_TYPE_HEALTH) {
          tile = 0;
        } else if(snapshot->data[0] == EntitySnapshot::STATION_TYPE_BLOW) {
          tile = 2;
        } else if(snapshot->data[0] == EntitySnapshot::STATION_TYPE_MORPH) {
          tile = 1;
        } else if(snapshot->data[0] == EntitySnapshot::STATION_TYPE_COMPOSITE) {
          tile = 3;
        }
        _objects[snapshot->id] = new Object(position, time, snapshot->id, snapshot->type, textures[EntitySnapshot::ENTITY_TYPE_STATION], tile);
        _objects[snapshot->id]->EnableInterpolation();
        _objects[snapshot->id]->visible = true;
        _objects[snapshot->id]->name_visible = false;
      } break;
    }
  }

  void OnEntityUpdate(const EntitySnapshot* snapshot) {
    CHECK(snapshot != NULL);

    TimeType time = snapshot->time;
    glm::vec2 position = glm::vec2(snapshot->x, snapshot->y);

    ObjectState state;
    state.position = position;
    state.health = static_cast<float>(snapshot->data[0]);
    state.blowCharge = static_cast<float>(snapshot->data[1]);
    state.morphCharge = static_cast<float>(snapshot->data[2]);

    if(snapshot->type == EntitySnapshot::ENTITY_TYPE_WALL) {
      _walls[snapshot->id]->UpdateCurrentState(state, time);
    } else {
      _objects[snapshot->id]->UpdateCurrentState(state, time);
    }
  }

  void OnPlayerUpdate(const EntitySnapshot* snapshot) {
    CHECK(snapshot != NULL);

    glm::vec2 position = glm::vec2(snapshot->x, snapshot->y);
    glm::vec2 distance = _player->GetPosition() - position;

    ObjectState state;
    state.position = position;
    _player_health = static_cast<float>(snapshot->data[0]);
    _player_blow_charge = static_cast<float>(snapshot->data[1]);
    _player_morph_charge = static_cast<float>(snapshot->data[2]);

    if(glm::length(distance) > _max_error) {
      _player->EnforceState(state, snapshot->time);
    } else {
      //_player->UpdateCurrentState(state, snapshot->time);
    }
  }

  bool OnEntityDisappearance(const EntitySnapshot* snapshot) {
    CHECK(snapshot != NULL);

    DeleteObject(snapshot->id);

    if(snapshot->type == EntitySnapshot::ENTITY_TYPE_BULLET) {
      // TODO[12.08.2012 xairy]: create explosion animation on explosion packet.
      // TODO[12.08.2012 xairy]: remove magic numbers;
      Animation* animation = new Animation();
      CHECK(animation != NULL);
      bool rv = animation->Initialize(textures[EntitySnapshot::ENTITY_TYPE_EXPLOSION], 30, false);
      if(rv == false) {
        return false;
      }
      //animation->SetPivot(glm::vec2(0.5f, 0.5f));
      animation->SetPosition(glm::vec2(snapshot->x, snapshot->y));
      animation->Play();
      _animations.push_back(animation);
    }

    return true;
  }

  bool _Loop() {
    if (_network_state == NETWORK_STATE_LOGGED_IN) {
      TimeType current_time = _GetTime();
      TimeType delta_time = current_time - _last_loop;
      _last_loop = current_time;

      glm::float_t delta_x = (_keyboard_state.right - _keyboard_state.left) * _client_options->speed * delta_time;
      glm::float_t delta_y = (_keyboard_state.down - _keyboard_state.up) * _client_options->speed * delta_time;

      bool intersection_x = false;
      bool intersection_y = false;

      std::map<int, Object*>::iterator it;
      glm::vec2 player_position = _player->GetPosition(current_time);

      for(it = _walls.begin() ; it != _walls.end(); it++) {
        glm::vec2 wall_position = it->second->GetPosition(current_time);
        if(abs(wall_position.x - (player_position.x + delta_x)) < (_player_size + _wall_size) / 2
          && abs(wall_position.y - (player_position.y)) < (_player_size + _wall_size) / 2) {
          intersection_x = true;
          break;
        }
      }
      for(it = _walls.begin() ; it != _walls.end(); it++) {
        glm::vec2 wall_position = it->second->GetPosition(current_time);
        if(abs(wall_position.x - (player_position.x)) < (_player_size + _wall_size) / 2
          && abs(wall_position.y - (player_position.y + delta_y)) < (_player_size + _wall_size) / 2) {
          intersection_y = true;
          break;
        }
      }

      if(!intersection_x) _player->Move(glm::vec2(delta_x, 0.0f));
      if(!intersection_y) _player->Move(glm::vec2(0.0f, delta_y));
    }

    return true;
  }

  // Returns approximate server time (with the correction).
  TimeType _GetTime() {
    return getTicks() + _time_correction;
  }

  // FIXME[xairy]: magic numbers.
  // TODO[xairy]: prettier HUD.
  void _RenderHUD() {
    // Sets origin to the left top corner.
    sf::Transform hud_transform;
    hud_transform.translate(_view.getCenter() - _view.getSize() / 2.0f);

    sf::RectangleShape health_rect(sf::Vector2f(100.0f * _player_health / _client_options->max_health, 10.0f));
    health_rect.setPosition(sf::Vector2f(50.0f, 510.0f));
    health_rect.setFillColor(sf::Color(0xFF, 0x00, 0xFF, 0xBB));
    _render_window->draw(health_rect, hud_transform);

    sf::RectangleShape blow_charge_rect(sf::Vector2f(100.0f * _player_blow_charge / _client_options->blow_capacity, 10.0f));
    blow_charge_rect.setPosition(sf::Vector2f(50.0f, 530.0f));
    blow_charge_rect.setFillColor(sf::Color(0x00, 0xFF, 0xFF, 0xBB));
    _render_window->draw(blow_charge_rect, hud_transform);

    sf::RectangleShape morph_charge_rect(sf::Vector2f(100.0f * _player_morph_charge / _client_options->morph_capacity, 10.0f));
    morph_charge_rect.setPosition(sf::Vector2f(50.0f, 550.0f));
    morph_charge_rect.setFillColor(sf::Color(0xFF, 0xFF, 0x00, 0xBB));
    _render_window->draw(morph_charge_rect, hud_transform);

    sf::CircleShape compass_border(60.0f);
    compass_border.setPosition(20.0f, 20.0f); // Not center.
    compass_border.setOutlineColor(sf::Color(0xFF, 0xFF, 0xFF, 0xFF));
    compass_border.setOutlineThickness(1.0f);
    compass_border.setFillColor(sf::Color(0xFF, 0xFF, 0xFF, 0x00));
    _render_window->draw(compass_border, hud_transform);

    TimeType render_time = _GetTime();

    std::map<int, Object*>::const_iterator it;
    for (it = _objects.begin(); it != _objects.end(); ++it) {
      Object* obj = it->second;

      glm::vec2 rel = obj->GetPosition(render_time) - _player->GetPosition(render_time);
      if (glm::length(rel) < 400) {
        rel = rel * (60.0f / 400.0f);
        sf::CircleShape circle(1.0f);
        circle.setPosition(sf::Vector2f(80.0f, 80.0f) + sf::Vector2f(rel.x, rel.y));
        circle.setFillColor(sf::Color(0xFF, 0x00, 0x00, 0xFF));
        _render_window->draw(circle, hud_transform);
      }
    }

    for (it = _walls.begin(); it != _walls.end(); ++it) {
      Object* obj = it->second;

      glm::vec2 rel = obj->GetPosition(render_time) - _player->GetPosition(render_time);
      if (glm::length(rel) < 400) {
        rel = rel * (60.0f / 400.0f);
        sf::CircleShape circle(1.0f);
        circle.setPosition(sf::Vector2f(80.0f, 80.0f) + sf::Vector2f(rel.x, rel.y));
        circle.setFillColor(sf::Color(0x00, 0xFF, 0x00, 0xFF));
        _render_window->draw(circle, hud_transform);
      }
    }

    {
      Object* obj = _player;

      glm::vec2 rel = obj->GetPosition(render_time) - _player->GetPosition(render_time);
      if (glm::length(rel) < 400) {
        rel = rel * (60.0f / 400.0f);
        sf::CircleShape circle(1.0f);
        circle.setPosition(sf::Vector2f(80.0f, 80.0f) + sf::Vector2f(rel.x, rel.y));
        circle.setFillColor(sf::Color(0x00, 0x00, 0xFF, 0xFF));
        _render_window->draw(circle, hud_transform);
      }
    }
  }

  // Renders everything.
  void _Render() {
    _render_window->clear();

    if (_network_state == NETWORK_STATE_LOGGED_IN) {
      TimeType render_time = _GetTime();
      glm::vec2 player_position = _player->GetPosition(render_time);

      _view.setCenter(glm::round(player_position.x), glm::round(player_position.y));
      _render_window->setView(_view);

      std::list<Animation*>::iterator it2;
      for(it2 = _animations.begin(); it2 != _animations.end();) {
        (*it2)->Render(*_render_window);
        if((*it2)->IsStopped()) {
          std::list<Animation*>::iterator it1 = it2;
          ++it1;
          delete *it2;
          _animations.erase(it2);
          it2 = it1;
        } else {
          ++it2;
        }
      }

      std::map<int,Object*>::iterator it;
      for(it = _walls.begin() ; it != _walls.end(); ++it) {
        RenderObject(it->second, render_time, textures, default_text_writer, *_render_window);
      }
      for(it = _objects.begin() ; it != _objects.end(); ++it) {
        RenderObject(it->second, render_time, textures, default_text_writer, *_render_window);
      }

      RenderObject(_player, render_time, textures, default_text_writer, *_render_window);

      _RenderHUD();
    }

    _render_window->display();
  }

  bool _is_running;

  sf::Font* default_text_writer;

  enet::Enet _enet;
  enet::ClientHost* _client;
  enet::Peer* _peer;
  enet::Event* _event;

  // In milliseconds.
  TimeType _connect_timeout;
  TimeType _time_correction;
  TimeType _last_tick;
  int _tick_rate;
  TimeType _last_loop;

  sf::RenderWindow* _render_window;
  sf::View _view;

  std::map<uint32_t, TextureAtlas*> textures;

  Object* _player;

  std::map<int, Object*> _objects;
  std::map<int, Object*> _walls;

  std::list<Animation*> _animations;

  ClientOptions* _client_options;

  uint32_t _wall_size;
  uint32_t _player_size;

  glm::float_t _max_error;

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

  IniFile::RecordMap settings;
};

int main(int argc, char** argv) {
  Application app;
  if(!app.Execute()) {
    Error::Print();
    app.Finalize();
    return EXIT_FAILURE;
  }
  app.Finalize();
  return EXIT_SUCCESS;
}
