// XXX[xairy]: linux fix.
#define __STDC_LIMIT_MACROS

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <algorithm>

#include <map>
#include <memory>
#include <list>
#include <string>

#include <GL/glew.h>
#include <SDL/SDL.h>
#include <glm/glm.hpp>
#include <FreeImage.h>

#include <base/error.hpp>
#include <base/macros.hpp>
#include <base/protocol.hpp>
#include <base/pstdint.hpp>

#include <enet-wrapper/enet.hpp>

#include <interpolator/interpolator.hpp>

#include <ini-file/ini_file.hpp>

#include "engine/animation.hpp"
#include "engine/render_window.hpp"
#include "engine/sprite.hpp"
#include "engine/texture_atlas.hpp"
#include "engine/text_writer.hpp"
#include "engine/canvas.hpp"

//#include <base/leak_detector.hpp>

#define _USE_MATH_DEFINES
#include <math.h>
static float fround(float f) {
  return ::floorf(f + 0.5f);
}

using namespace bm;

void Warning(const char* fmt, ...) {
  CHECK(fmt != NULL);

  char buf[1024];

  va_list args;
  va_start(args, fmt);
  vsnprintf(buf, sizeof(buf) - 1, fmt, args);
  va_end(args);

  fprintf(stderr, "WARN: %s\n", buf);
}

// Appends packet type and data to the end of the buffer.
template<class T> void AppendPacketToBuffer(std::vector<char>& buf, const T* data, Packet::Type packet_type) {
  CHECK(data != NULL);
  CHECK(Packet::TYPE_UNKNOWN <= packet_type && packet_type <= Packet::TYPE_MAX_VALUE);

  // Append packet type.
  buf.insert(buf.end(), 
    reinterpret_cast<const char*>(&packet_type),
    reinterpret_cast<const char*>(&packet_type) + sizeof(packet_type));

  // Append packet data.
  buf.insert(buf.end(), 
    reinterpret_cast<const char*>(data),
    reinterpret_cast<const char*>(data) + sizeof(*data));
}

// Attempts to synchronously disconnect the peer.
bool DisconnectPeer(bm::Peer* peer, bm::Event* event, bm::ClientHost* host, uint32_t timeout) {
  CHECK(peer != NULL);
  CHECK(event != NULL);
  CHECK(host != NULL);

  peer->Disconnect();

  uint32_t start = SDL_GetTicks();

  while(SDL_GetTicks() - start <= timeout) {
    bool rv = host->Service(event, (uint32_t) timeout);
    if(rv == false) {
      return false;
    }

    if(event != NULL && event->GetType() == Event::EVENT_DISCONNECT) {
      return true;
    }
  }

  return false;
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

typedef interpolator::LinearInterpolator<ObjectState, TimeType> ObjectInterpolator;

// TODO[24.7.2012 alex]: fix method names
struct Object {
  // FIXME[18.11.2012 alex]: hardcoded initial interpolation time step.
  Object(const glm::vec2& position, TimeType time, uint32_t id, uint32_t type)
    : id(id), type(type), visible(false), interpolation_enabled(false),
    name_visible(false), interpolator(ObjectInterpolator(TimeType(75), 1)), tile(0)
  {
    ObjectState state;
    state.blowCharge = 0;
    state.health = 0;
    state.morphCharge = 0;
    state.position = position;
    interpolator.Push(state, time);
    
    name_render_offset = glm::vec2(-8.0f, -20.0f);
  }
  
  void EnableInterpolation() {
    interpolation_enabled = true;
    interpolator.SetFrameCount(2);
  }

  void DisableInterpolation() {
    interpolation_enabled = false;
    interpolator.SetFrameCount(1);
  }
  
  void EnforceState(const ObjectState& state, TimeType time) {
    interpolator.Clear();
    interpolator.Push(state, time);
  }
  
  void UpdateCurrentState(const ObjectState& state, TimeType time) {
    interpolator.Push(state, time);
  }  
  
  glm::vec2 GetPosition(TimeType time) {
    return interpolator.Interpolate(time).position;
  }

  glm::vec2 GetPosition() {
    assert(!interpolation_enabled);
    return GetPosition(TimeType(0));
  }

  void SetPosition(const glm::vec2& value) {
    assert(!interpolation_enabled);
    ObjectState state = interpolator.Interpolate(TimeType(0));
    state.position = value;
    EnforceState(state, TimeType(0));
  }

  void Move(const glm::vec2& value) {
    assert(!interpolation_enabled);
    ObjectState state = interpolator.Interpolate(TimeType(0));
    state.position = state.position + value;
    EnforceState(state, TimeType(0));
  }

  uint32_t id;
  uint32_t type;
  size_t tile;

  bool visible;
  
  bool name_visible;
  glm::vec2 name_render_offset;

  bool interpolation_enabled;
  ObjectInterpolator interpolator;
};

void RenderObject(Object* object, TimeType time, std::map<uint32_t, TextureAtlas*> textures,
                  TextWriter* text_writer, Canvas* canvas) {
  CHECK(object != NULL);
  CHECK(text_writer != NULL);

  ObjectState state = object->interpolator.Interpolate(time);

  if (object->visible) {
    std::map<uint32_t, TextureAtlas*>::const_iterator it = textures.find(object->type);
    if (it != textures.end()) {
      bm::TextureAtlas* atlas = it->second;
      canvas->DrawTexturedQuad(state.position, glm::vec2(0.5f, 0.5f), 0.0f, atlas, object->tile);
    }
  }

  if (object->name_visible) {
    glm::vec2 caption_position = glm::round(state.position + object->name_render_offset);
    text_writer->PrintText(glm::vec4(1, 1, 1, 1), caption_position.x, caption_position.y,
      "%u (%.2f,%.2f)", object->id, state.position.x, state.position.y);
  }
}

template<class T>
struct Rect {
  T left;
  T right;
  T top;
  T bottom;

  Rect() : left(0), right(0), top(0), bottom(0) { }
  Rect(T left, T right, T bottom, T top) : left(left), right(right), top(top), bottom(bottom) {
    CHECK(bottom <= top);
    CHECK(left <= right);
  }
};

template<class T> bool IsInside(const Rect<T>& rect, T x, T y) {
  return (rect.left <= x && x <= rect.right) &&
    (rect.bottom <= y && y <= rect.top);
}

class Game {
public:
  void Update();
  void Render();
};

class Application {
public:
  Application() : _state(STATE_FINALIZED), _network_state(NETWORK_STATE_DISCONNECTED) { }
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
        _SendInputEvents();
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
    if(_peer != NULL) delete _peer;
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

    //_render_window.Finalize();
    
    default_text_writer->Destroy();
    delete default_text_writer;

    _state = STATE_FINALIZED;
  }

private:
  struct MenuItem {
    std::string text;
    void (Application::*buttonHandler)(MenuItem* menuItem);

    MenuItem(const std::string& text) : text(text) { }
  };
  struct Menu {
    std::vector<MenuItem> items;
    size_t selected;
  };

  Menu _menu;
  TextWriter _menuFont;

  void RenderMenu(Menu& menu) {
    const float ITEM_HEIGHT = 30.0f;
    const float ITEM_WIDTH = 200.0f;
    
    _canvas.SetCoordinateType(Canvas::PixelsFlipped);
    
    // Move the coordinate axes to the center of the screen.
    glm::vec2 screenSize = _canvas.GetSize();
    glm::mat3x3 centerTranslation(1, 0, screenSize.x / 2,
                                  0, 1, screenSize.y / 2,
                                  0, 0,                1);
    // Transpose since glm keeps matrices in a column-major order.
    centerTranslation = glm::transpose(centerTranslation);
    _canvas.SetTransform(centerTranslation);
    
    size_t itemCount = menu.items.size();
    float firstY = itemCount * ITEM_HEIGHT / 2;
    for (size_t i = 0; i < itemCount; i++) {


      MenuItem item = menu.items[i];
      
      glm::vec2 pos = glm::vec2(-ITEM_WIDTH / 2, firstY - i * ITEM_HEIGHT);
      glm::vec2 size = glm::vec2(ITEM_WIDTH, ITEM_HEIGHT);

      // Select color for the item.
      glm::vec4 clr(1, 1, 1, 1);
      if (menu.selected == i) {
        clr = glm::vec4(1, 0, 0, 1);
      }

      _canvas.DrawRect(clr, pos, size);
      _menuFont.PrintText(clr, pos.x, pos.y - size.y, item.text.c_str());
    }
  }
  
  void InitMenu() {
    _menu.items.push_back(MenuItem("Test"));
    _menu.items.push_back(MenuItem("Test2"));
    _menu.items.push_back(MenuItem("Test3"));
    _menu.selected = 0;
    
    _menuFont.InitFont("data/fonts/tahoma.ttf", 12);
  }

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
    
    InitMenu();
    
    default_text_writer = new TextWriter();
    default_text_writer->InitFont("data/fonts/tahoma.ttf", 12);

    _state = STATE_INITIALIZED;

    return true;
  }

  bool _InitializeGraphics() {
    if (!_render_window.Initialize("Blowmorph", 600, 600, false)) {
      return false;
    }
    
    // Init 2D canvas.
    if (!_canvas.Init()) {
      return false;
    }

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

    std::auto_ptr<ClientHost> client(_enet.CreateClientHost());
    if(client.get() == NULL) {
      return false;
    }

    std::string host = IniFile::GetValue<std::string>(settings, "server.host", "127.0.0.1");
    uint16_t port = IniFile::GetValue(settings, "server.port", 4242);

    std::auto_ptr<Peer> peer(client->Connect(host, port));
    if(peer.get() == NULL) {
      return false;
    }

    std::auto_ptr<Event> event(_enet.CreateEvent());
    if(event.get() == NULL) {
      return false;
    }

    CHECK(0 <= _connect_timeout && _connect_timeout <= UINT32_MAX);
    bool rv = client->Service(event.get(), (uint32_t) _connect_timeout);
    if(rv == false) {
      return false;
    }
    if(event->GetType() != Event::EVENT_CONNECT) {
      BM_ERROR("Could not connect to server.");
      return false;
    }
    _client = client.release();
    _peer = peer.release();
    _event = event.release();

    _network_state = NETWORK_STATE_CONNECTED;

    printf("Connected to %s:%u.\n", _event->GetPeerIp().c_str(), _event->GetPeerPort());

    return true;
  }

  bool _PumpEvents() {
    SDL_Event event;
    while(SDL_PollEvent(&event)) {
      if(!_ProcessEvent(&event)) {
        return false;
      }
    }
    return true;
  }

  bool _ProcessEvent(SDL_Event* event) {
    switch(event->type) {
      case SDL_KEYDOWN: {
        KeyboardEvent keyboard_event;
        keyboard_event.time = _GetTime();
        keyboard_event.event_type = KeyboardEvent::EVENT_KEYDOWN;
        switch(event->key.keysym.sym) {
          case SDLK_a: {
            _keyboard_state.left = true;
            keyboard_event.key_type = KeyboardEvent::KEY_LEFT;
            _keyboard_events.push_back(keyboard_event);
            break;
          }
          case SDLK_d: {
            _keyboard_state.right = true;
            keyboard_event.key_type = KeyboardEvent::KEY_RIGHT;
            _keyboard_events.push_back(keyboard_event);
            break;
          }
          case SDLK_w: {
            _keyboard_state.up = true;
            keyboard_event.key_type = KeyboardEvent::KEY_UP;
            _keyboard_events.push_back(keyboard_event);
            break;
          }
          case SDLK_s: {
            _keyboard_state.down = true;
            keyboard_event.key_type = KeyboardEvent::KEY_DOWN;
            _keyboard_events.push_back(keyboard_event);
            break;
          }
          case SDLK_ESCAPE: {
            _is_running = false;
            
            CHECK(0 <= _connect_timeout && _connect_timeout <= UINT32_MAX);
            if (!DisconnectPeer(_peer, _event, _client, (uint32_t) _connect_timeout)) {
              BM_ERROR("Did not receive EVENT_DISCONNECT event while disconnecting.\n");
              return false;
            } else {
              printf("Client disconnected.\n");
            }
            
            break;
          }
        }
        break;
      }

      case SDL_KEYUP: {
        KeyboardEvent keyboard_event;
        keyboard_event.time = _GetTime();
        keyboard_event.event_type = KeyboardEvent::EVENT_KEYUP;
        switch(event->key.keysym.sym) {
          case SDLK_a:
            _keyboard_state.left = false;
            keyboard_event.key_type = KeyboardEvent::KEY_LEFT;
            _keyboard_events.push_back(keyboard_event);
            break;
          case SDLK_d:
            _keyboard_state.right = false;
            keyboard_event.key_type = KeyboardEvent::KEY_RIGHT;
            _keyboard_events.push_back(keyboard_event);
            break;
          case SDLK_w:
            _keyboard_state.up = false;
            keyboard_event.key_type = KeyboardEvent::KEY_UP;
            _keyboard_events.push_back(keyboard_event);
            break;
          case SDLK_s:
            _keyboard_state.down = false;
            keyboard_event.key_type = KeyboardEvent::KEY_DOWN;
            _keyboard_events.push_back(keyboard_event);
            break;
        }
        break;
      }

      case SDL_QUIT: {
        _is_running = false;
        
        CHECK(0 <= _connect_timeout && _connect_timeout <= UINT32_MAX);
        if (!DisconnectPeer(_peer, _event, _client, (uint32_t) _connect_timeout)) {
          BM_ERROR("Did not receive EVENT_DISCONNECT event while disconnecting.\n");
          return false;
        } else {
          printf("Client disconnected.\n");
        }
        
        break;
      }

      case SDL_MOUSEMOTION: {
        break;
      }

      case SDL_MOUSEBUTTONDOWN: {
        MouseEvent mouse_event;
        mouse_event.time = _GetTime();
        if(event->button.button == SDL_BUTTON_LEFT) {
          mouse_event.button_type = MouseEvent::BUTTON_LEFT;
        } else {
          mouse_event.button_type = MouseEvent::BUTTON_RIGHT;
        }
        mouse_event.event_type = MouseEvent::EVENT_KEYDOWN;
        
        int screenWidth = _render_window.GetWidth();
        int screenHeight = _render_window.GetHeight();
        mouse_event.x = (event->button.x - screenWidth / 2) + _player->GetPosition().x;
        mouse_event.y = (event->button.y - screenHeight / 2) + _player->GetPosition().y;
        _mouse_events.push_back(mouse_event);
        break;
      }
        
      case SDL_MOUSEBUTTONUP: {
        MouseEvent mouse_event;
        mouse_event.time = _GetTime();
        if(event->button.button == SDL_BUTTON_LEFT) {
          mouse_event.button_type = MouseEvent::BUTTON_LEFT;
        } else {
          mouse_event.button_type = MouseEvent::BUTTON_RIGHT;
        }
        mouse_event.event_type = MouseEvent::EVENT_KEYUP;
        int screenWidth = _render_window.GetWidth();
        int screenHeight = _render_window.GetHeight();
        mouse_event.x = (event->button.x - screenWidth / 2) + _player->GetPosition().x;
        mouse_event.y = (event->button.y - screenHeight / 2) + _player->GetPosition().y;
        _mouse_events.push_back(mouse_event);
        break;
      }
    }

    return true;
  }

  bool _PumpPackets(uint32_t timeout) {
    std::vector<char> message;
    
    uint32_t start_time = SDL_GetTicks();
    do {
      uint32_t time = SDL_GetTicks();
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
        case Event::EVENT_RECEIVE: {
          _event->GetData(&message);
          _event->DestroyPacket();

          const Packet::Type* type = reinterpret_cast<Packet::Type*>(&message[0]);
          const void* data = reinterpret_cast<void*>(&message[0] + sizeof(Packet::Type)); 
          size_t len = message.size() - sizeof(Packet::Type);
          
          //printf("Received %d.", *type);
          
          // XXX[24.7.2012 alex]: will only process single packet at a time
          ProcessPacket(*type, data, len);
        } break;
        
        case Event::EVENT_CONNECT: {
          Warning("Got EVENT_CONNECT while being already connected.");
        } break;

        case Event::EVENT_DISCONNECT: {
          _network_state = NETWORK_STATE_DISCONNECTED;
          _is_running = false;
          BM_ERROR("Connection lost.");
          return false;
        } break;
      }
    } while (_event->GetType() != Event::EVENT_NONE);
    
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
        Warning("Received a packet while being in disconnected state.");
        return true;
      } break;
      case NETWORK_STATE_CONNECTED: {
        if (type != Packet::TYPE_CLIENT_OPTIONS) {
          Warning("Received packet with a type %d while waiting for client options.", type);
        } else if (len != sizeof(ClientOptions)) {
          Warning("Received packet has incorrect length.");
        } else {
          const ClientOptions* options = reinterpret_cast<const ClientOptions*>(data); 
          _client_options = new ClientOptions(*options);
          
          // Switch to a new state.
          _network_state = NETWORK_STATE_SYNCHRONIZATION;
          
          // Send a time synchronization request.
          TimeSyncData request_data;
          request_data.client_time = TimeType(SDL_GetTicks());
          
          std::vector<char> buf;
          AppendPacketToBuffer(buf, &request_data, Packet::TYPE_SYNC_TIME_REQUEST);

          bool rv = _peer->Send(&buf[0], buf.size(), true);
          if(rv == false) {
            return false;
          }
        }
        return true;
      } break;
      case NETWORK_STATE_SYNCHRONIZATION: {
        if (type != Packet::TYPE_SYNC_TIME_RESPONSE) {
          Warning("Received packet with a type %d while waiting for time sync response.", type);
        } else if (len != sizeof(TimeSyncData)) {
          Warning("Received packet has incorrect length.");
        } else {
          const TimeSyncData* response_data = reinterpret_cast<const TimeSyncData*>(data);
          // Calculate the time correction.
          TimeType client_time = TimeType(SDL_GetTicks()); 
          TimeType latency = (client_time - response_data->client_time) / 2;
          _time_correction = response_data->server_time + latency - client_time;

          printf("Time correction is %lld ms.\n", _time_correction);
          printf("Latency is %lld.\n", latency);

          _network_state = NETWORK_STATE_LOGGED_IN;
          
          // XXX[24.7.2012 alex]: move it to the ProcessPacket method
          _player = new Object(glm::vec2(_client_options->x, _client_options->y), TimeType(0), 
                               _client_options->id, EntitySnapshot::ENTITY_TYPE_PLAYER);
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
          Warning("Received packet is not an entity snapshot.");
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
        _walls[snapshot->id] = new Object(position, time, snapshot->id, snapshot->type);
        size_t tile;
        if(snapshot->data[0] == EntitySnapshot::WALL_TYPE_ORDINARY) {
          tile = 3;
        } else if(snapshot->data[0] == EntitySnapshot::WALL_TYPE_UNBREAKABLE) {
          tile = 2;
        } else if(snapshot->data[0] == EntitySnapshot::WALL_TYPE_MORPHED) {
          tile = 1;
        }
        _walls[snapshot->id]->tile = tile;
        _walls[snapshot->id]->EnableInterpolation();
        _walls[snapshot->id]->visible = true;
        _walls[snapshot->id]->name_visible = false;
      } break;

      case EntitySnapshot::ENTITY_TYPE_BULLET: {
        _objects[snapshot->id] = new Object(position, time, snapshot->id, snapshot->type);
        _objects[snapshot->id]->EnableInterpolation();
        _objects[snapshot->id]->visible = true;
        _objects[snapshot->id]->name_visible = false;
      } break;

      case EntitySnapshot::ENTITY_TYPE_PLAYER: {
        _objects[snapshot->id] = new Object(position, time, snapshot->id, snapshot->type);
        _objects[snapshot->id]->EnableInterpolation();
        _objects[snapshot->id]->visible = true;
        _objects[snapshot->id]->name_visible = true;
      } break;

      case EntitySnapshot::ENTITY_TYPE_DUMMY: {
        _objects[snapshot->id] = new Object(position, time, snapshot->id, snapshot->type);
        _objects[snapshot->id]->EnableInterpolation();
        _objects[snapshot->id]->visible = true;
        _objects[snapshot->id]->name_visible = false;
      } break;

      case EntitySnapshot::ENTITY_TYPE_STATION: {
        _objects[snapshot->id] = new Object(position, time, snapshot->id, snapshot->type);
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
        _objects[snapshot->id]->tile = tile;
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
      bool rv = animation->Initialize(textures[EntitySnapshot::ENTITY_TYPE_EXPLOSION], 30);
      if(rv == false) {
        return false;
      }
      animation->SetPivot(glm::vec2(0.5f, 0.5f));
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

  // Sends input events to the server and 
  // clears the input event queues afterwards.
  bool _SendInputEvents() {
    std::vector<char> buf;

    for(size_t i = 0; i < _keyboard_events.size(); i++) {
      buf.clear();
      
      AppendPacketToBuffer(buf, &_keyboard_events[i], Packet::TYPE_KEYBOARD_EVENT);
      
      bool rv = _peer->Send(&buf[0], buf.size());
      if(rv == false) {
        return false;
      }
    }
    _keyboard_events.clear();
    
    for(size_t i = 0; i < _mouse_events.size(); i++) {
      buf.clear();
      
      AppendPacketToBuffer(buf, &_mouse_events[i], Packet::TYPE_MOUSE_EVENT);
      
      bool rv = _peer->Send(&buf[0], buf.size());
      if(rv == false) {
        return false;
      }
    }
    _mouse_events.clear();

    return true;
  }

  // Returns approximate server time (with the correction).
  TimeType _GetTime() {
    return TimeType(SDL_GetTicks()) + _time_correction;
  }
  
  void _RenderHUD() {
    _canvas.SetCoordinateType(Canvas::PixelsFlipped);
    _canvas.FillRect(glm::vec4(1, 0, 1, 0.8), glm::vec2(50, 90), glm::vec2(100 * _player_health / _client_options->max_health, 10));
    _canvas.FillRect(glm::vec4(0, 1, 1, 0.8), glm::vec2(50, 70), glm::vec2(100 * _player_blow_charge / _client_options->blow_capacity, 10));
    _canvas.FillRect(glm::vec4(1, 1, 0, 0.8), glm::vec2(50, 50), glm::vec2(100 * _player_morph_charge / _client_options->morph_capacity, 10));
  
    _canvas.SetCoordinateType(Canvas::Pixels);
    _canvas.DrawCircle(glm::vec4(1, 1, 1, 1), glm::vec2(80, 80), 60, 20);
    
    TimeType render_time = _GetTime();
    
    std::map<int, Object*>::const_iterator it;
    for (it = _objects.begin(); it != _objects.end(); ++it) {
      Object* obj = it->second;
      
      glm::vec2 rel = obj->GetPosition(render_time) - _player->GetPosition(render_time);
      if (glm::length(rel) < 400) {
        rel = rel * (60.0f / 400.0f);
        _canvas.DrawCircle(glm::vec4(1, 0, 0, 1), glm::vec2(80, 80) + rel, 1, 6);
      }
    }
    
    for (it = _walls.begin(); it != _walls.end(); ++it) {
      Object* obj = it->second;

      glm::vec2 rel = obj->GetPosition(render_time) - _player->GetPosition(render_time);
      if (glm::length(rel) < 400) {
        rel = rel * (60.0f / 400.0f);
        _canvas.DrawCircle(glm::vec4(0, 1, 0, 1), glm::vec2(80, 80) + rel, 1, 6);
      }
    }
    
    {
      Object* obj = _player;

      glm::vec2 rel = obj->GetPosition(render_time) - _player->GetPosition(render_time);
      if (glm::length(rel) < 400) {
        rel = rel * (60.0f / 400.0f);
        _canvas.DrawCircle(glm::vec4(0, 0, 1, 1), glm::vec2(80, 80) + rel, 1, 6);
      }
    }
  }

  // Renders everything.
  void _Render() {
    glClear(GL_COLOR_BUFFER_BIT);
    
    if (_network_state == NETWORK_STATE_LOGGED_IN) {  
      _canvas.SetCoordinateType(Canvas::Pixels);
      
      glMatrixMode(GL_MODELVIEW);
      glLoadIdentity();
      
      TimeType render_time = _GetTime();
      glm::vec2 player_position = _player->GetPosition(render_time);
      
      int screenWidth = _render_window.GetWidth();
      int screenHeight = _render_window.GetHeight();
      
      glTranslatef(::fround(screenWidth / 2.0f - player_position.x), 
                   ::fround(screenHeight / 2.0f - player_position.y), 0);

      std::list<Animation*>::iterator it2;
      for(it2 = _animations.begin(); it2 != _animations.end();) {
        (*it2)->Render();
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
        RenderObject(it->second, render_time, textures, default_text_writer, &_canvas);
      }
      for(it = _objects.begin() ; it != _objects.end(); ++it) {
        RenderObject(it->second, render_time, textures, default_text_writer, &_canvas);
      }

      RenderObject(_player, render_time, textures, default_text_writer, &_canvas);
      
      _RenderHUD();
      //RenderMenu(_menu);

      _render_window.SwapBuffers();
    }
  }

  bool _is_running;
  
  TextWriter* default_text_writer;

  Enet _enet;
  ClientHost* _client;
  Peer* _peer;
  Event* _event;

  // In milliseconds.
  TimeType _connect_timeout;
  TimeType _time_correction;
  TimeType _last_tick;
  int _tick_rate;
  TimeType _last_loop;

  RenderWindow _render_window;
  Canvas _canvas;

  // XXX[24.7.2012 alex]: copypasta
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
    //bm::leak_detector::PrintAllLeaks();
    //while(true);
    return EXIT_FAILURE;
  }
  app.Finalize();
  //bm::leak_detector::PrintAllLeaks();
  return EXIT_SUCCESS;
}

/*
#include "game_controller.hpp"
#include "network_controller.hpp"
#include "packet_processer.hpp"
#include "window.hpp"

int main(int argc, char** argv) {
  NetworkController nc;
  bool rv = nc.Initialize("127.0.0.1", 4242);
  CHECK(rv == true);
  rv = nc.Connect(500);
  CHECK(rv == true);

  PacketProcesser pp;
  GameController gc;
  Window ww;

  rv = pp.Initialize(&nc, &gc);
  CHECK(rv == true);

  rv = gc.Initialize(&pp, &ww);
  CHECK(rv == true);

  rv = ww.Initialize(NULL);
  CHECK(rv == true);

  while(true) {
    rv = nc.Service();
    CHECK(rv == true);
    rv = gc.Update();
    CHECK(rv == true);
    rv = ww.Render();
    CHECK(rv == true);
    rv = ww.PumpEvents();
    CHECK(rv == true);
  }

  nc.Finalize();
  rv = pp.Finalize();
  CHECK(rv == true);
  rv = gc.Finalize();
  CHECK(rv == true);
  ww.Finalize();

  return EXIT_SUCCESS;
}
*/
