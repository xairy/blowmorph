#include <cstdio>
#include <cstdlib>
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

using namespace bm;

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

typedef interpolator::LinearInterpolator<ObjectState, uint32_t> ObjectInterpolator;

// TODO[24.7.2012 alex]: fix method names
class Object {
public:
  Object(const glm::vec2& position, uint32_t time, int id)
    : _id(id), _sprite_set(false), _interpolation_enabled(false),
    _caption_enabled(false), _interpolator(ObjectInterpolator(75))
  {
    _current_state.position = position;
    _interpolator.Push(_current_state, time);
  }

  ~Object() {
    if(_caption_enabled) {
      _text_writer.Destroy();
    }
  };

  int GetId() const {
    return _id;
  }

  bool SetSprite(TextureAtlas* texture, size_t tile = 0) {
    bool rv = _sprite.Init(texture, tile);
    if(rv == false) {
      return false;
    }
    _sprite.SetPosition(_current_state.position);
    _sprite_set = true;
    return true;
  }

  void ResetSprite() {
    _sprite_set = false;
  }

  void EnableInterpolation() {
    _interpolation_enabled = true;
  }

  void DisableInterpolation() {
    _interpolation_enabled = false;
  }

  // TODO: think about CHECK's and stuff.
  bool EnableCaption() {
    CHECK(_caption_enabled == false);
    bool rv = _text_writer.InitFont("data/fonts/tahoma.ttf", 10);
    if(rv == false) {
      return false;
    }
    _caption_pivot = glm::vec2(-8.0f, -20.0f);
    _caption_enabled = true;
    return true;
  }

  void DisableCaption() {
    CHECK(_caption_enabled == true);
    _text_writer.Destroy();
    _caption_enabled = false;
  }

  void UpdateCurrentState(const ObjectState& state, uint32_t time) {
    if(_interpolation_enabled) {
      _interpolator.Push(state, time);
    }
    _current_state = state;
  }
  
  void EnforceState(const ObjectState& state, uint32_t time) {
    _interpolator.Clear();
    _interpolator.Push(state, time);
    _current_state = state;
  }

  void Render(uint32_t time) {
    if(_interpolation_enabled) {
      _current_state = _interpolator.Interpolate(time);
    }

    if(_sprite_set) {
      _sprite.SetPosition(_current_state.position);
      _sprite.Render();
    }

    if(_caption_enabled) {
      glm::vec2 caption_position = _current_state.position + _caption_pivot;
      _text_writer.PrintText(glm::vec4(1, 1, 1, 1), caption_position.x, caption_position.y,
        "%u (%.2f,%.2f)", _id, _current_state.position.x, _current_state.position.y);
    }
  }

  void SetPosition(const glm::vec2& value) {
    _current_state.position = value;
  }

  glm::vec2 GetPosition() const {
    return _current_state.position;
  }

  void Move(const glm::vec2& value) {
    _current_state.position += value;
  }

  void SetPivot(const glm::vec2& value) {
    CHECK(_sprite_set == true);
    _sprite.SetPivot(value);
  }

private:
  // TODO: make it 'bm::uint32_t'.
  int _id;

  bool _sprite_set;
  Sprite _sprite;
  
  bool _caption_enabled;
  TextWriter _text_writer;
  glm::vec2 _caption_pivot;

  bool _interpolation_enabled;
  ObjectInterpolator _interpolator;
  ObjectState _current_state;
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

      uint32_t current_time = _GetTime();
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
    if (_player_texture != NULL) delete _player_texture;
    if (_bullet_texture != NULL) delete _bullet_texture;
    if (_wall_texture != NULL) delete _wall_texture;
    if (_dummy_texture != NULL) delete _dummy_texture;
    if (_explosion_texture != NULL) delete _explosion_texture;
    if (_station_texture != NULL) delete _station_texture;

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

    _resolution_x = 600;
    _resolution_y = 600;

    _player_texture = NULL;
    _bullet_texture = NULL;
    _wall_texture = NULL;
    _dummy_texture = NULL;
    _explosion_texture = NULL;
    _station_texture = NULL;

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

    _state = STATE_INITIALIZED;

    return true;
  }

  bool _InitializeGraphics() {
    if (!_render_window.Initialize("Blowmorph", 600, 600, false)) {
      return false;
    }
    
    if (!_canvas.Init()) {
      return false;
    }

    // XXX[24.7.2012 alex]: awful lot of copypasta
    _player_texture = bm::LoadOldTexture("data/images/player.png", 0);
    if(_player_texture == NULL) {
      return false;
    }

    _bullet_texture = bm::LoadOldTexture("data/images/bullet.png", (8 << 16) + (54 << 8) + 129);
    if(_bullet_texture == NULL) {
      return false;
    }
    
    _wall_texture = bm::LoadTileset("data/images/walls.png", 0, 1, 1, 17, 17, 16, 16);
    if(_wall_texture == NULL) {
      return false;
    }

    _dummy_texture = bm::LoadOldTexture("data/images/guy.png", 0);
    if(_dummy_texture == NULL) {
      return false;
    }

    _explosion_texture = bm::LoadTileset("data/images/explosion.png", 0, 1, 1, 61, 61, 60, 60);
    if(_explosion_texture == NULL) {
      return false;
    }

    _station_texture = bm::LoadTileset("data/images/kits.png", 0, 1, 1, 31, 31, 30, 30);
    if(_station_texture == NULL) {
      return false;
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

    bool rv = client->Service(event.get(), _connect_timeout);
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
            if(!_Disconnect()) {
              return false;
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
          case SDLK_ESCAPE:
            _is_running = false;
            if(!_Disconnect()) {
              return false;
            }
            break;
        }
        break;
      }

      case SDL_QUIT: {
        if(!_Disconnect()) {
          return false;
        }
        _is_running = false;
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
        mouse_event.x = (event->button.x - _resolution_x / 2) + _player->GetPosition().x;
        mouse_event.y = (event->button.y - _resolution_y / 2) + _player->GetPosition().y;
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
        mouse_event.x = (event->button.x - _resolution_x / 2) + _player->GetPosition().x;
        mouse_event.y = (event->button.y - _resolution_y / 2) + _player->GetPosition().y;
        _mouse_events.push_back(mouse_event);
        break;
      }
    }

    return true;
  }

  void Warning(const char* fmt, ...) {
    char buf[1024];
    
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf) - 1, fmt, args);
    va_end(args);
    
    fprintf(stderr, "WARN: %s\n", buf);
  }

  bool _PumpPackets(uint32_t timeout) {
    std::vector<char> message;
    
    do {
      bool rv = _client->Service(_event, timeout);
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
          request_data.client_time = SDL_GetTicks();
          Packet::Type request_type = Packet::TYPE_SYNC_TIME_REQUEST;

          std::vector<char> message;
          message.insert(message.end(), reinterpret_cast<char*>(&request_type),
            reinterpret_cast<char*>(&request_type) + sizeof(request_type));
          message.insert(message.end(), reinterpret_cast<char*>(&request_data),
            reinterpret_cast<char*>(&request_data) + sizeof(request_data));

          bool rv = _peer->Send(&message[0], message.size(), true);
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
          uint32_t client_time = SDL_GetTicks(); 
          uint32_t latency = (client_time - response_data->client_time) / 2;
          _time_correction = response_data->server_time + latency - client_time;

          printf("Time correction is %u ms.\n", _time_correction);
          printf("Latency is %u.\n", latency);

          _network_state = NETWORK_STATE_LOGGED_IN;
          
          // XXX[24.7.2012 alex]: move it to the ProcessPacket method
          _player = new Object(glm::vec2(_client_options->x, _client_options->y), 0, _client_options->id);
          if(_player == NULL) {
            BM_ERROR("Unable to allocate memory!");
            return false;
          }
          // XXX[24.7.2012 alex]: maybe we should have a xml file for each object with
          //                      texture paths, pivots, captions, etc
          _player->SetSprite(_player_texture);
          _player->SetPosition(glm::vec2(_client_options->x, _client_options->y));
          _player->SetPivot(glm::vec2(0.5f, 0.5f));
          _player->EnableCaption();
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
            DeleteObject(snapshot->id);

            if(snapshot->type == EntitySnapshot::ENTITY_TYPE_BULLET) {
              // TODO[12.08.2012 xairy]: create explosion animation on explosion packet.
              // TODO[12.08.2012 xairy]: remove magic numbers;
              Animation* animation = new Animation();
              if(animation == NULL) {
                BM_ERROR("Unable to allocate memory!");
                return false;
              }
              bool rv = animation->Initialize(_explosion_texture, 30);
              if(rv == false) {
                return false;
              }
              animation->SetPivot(glm::vec2(0.5f, 0.5f));
              animation->SetPosition(glm::vec2(snapshot->x, snapshot->y));
              animation->Play();
              _animations.push_back(animation);
            }
          } else {
            glm::vec2 position = glm::vec2(snapshot->x, snapshot->y);
            uint32_t time = snapshot->time;

            if(snapshot->id == _player->GetId()) {
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
              
              break;
            }

            if(_objects.count(snapshot->id) > 0 || _walls.count(snapshot->id) > 0) {
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
            } else {
              switch(snapshot->type) {
                case EntitySnapshot::ENTITY_TYPE_WALL: {
                  _walls[snapshot->id] = new Object(position, time, snapshot->id);
                  size_t tile;
                  if(snapshot->data[0] == EntitySnapshot::WALL_TYPE_ORDINARY) {
                    tile = 3;
                  } else if(snapshot->data[0] == EntitySnapshot::WALL_TYPE_UNBREAKABLE) {
                    tile = 2;
                  } else if(snapshot->data[0] == EntitySnapshot::WALL_TYPE_MORPHED) {
                    tile = 1;
                  }
                  _walls[snapshot->id]->SetSprite(_wall_texture, tile);
                  _walls[snapshot->id]->EnableInterpolation();
                  _walls[snapshot->id]->SetPivot(glm::vec2(0.5f, 0.5f));
                } break;

                case EntitySnapshot::ENTITY_TYPE_BULLET: {
                  _objects[snapshot->id] = new Object(position, time, snapshot->id);
                  _objects[snapshot->id]->SetSprite(_bullet_texture);
                  _objects[snapshot->id]->EnableInterpolation();
                  _objects[snapshot->id]->SetPivot(glm::vec2(0.5f, 0.5f));
                } break;

                case EntitySnapshot::ENTITY_TYPE_PLAYER: {
                  _objects[snapshot->id] = new Object(position, time, snapshot->id);
                  _objects[snapshot->id]->SetSprite(_player_texture);
                  _objects[snapshot->id]->EnableInterpolation();
                  _objects[snapshot->id]->SetPivot(glm::vec2(0.5f, 0.5f));
                  _objects[snapshot->id]->EnableCaption();
                } break;

                case EntitySnapshot::ENTITY_TYPE_DUMMY: {
                  _objects[snapshot->id] = new Object(position, time, snapshot->id);
                  _objects[snapshot->id]->SetSprite(_dummy_texture);
                  _objects[snapshot->id]->EnableInterpolation();
                  _objects[snapshot->id]->SetPivot(glm::vec2(0.5f, 0.5f));
                } break;

                case EntitySnapshot::ENTITY_TYPE_STATION: {
                  _objects[snapshot->id] = new Object(position, time, snapshot->id);
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
                  _objects[snapshot->id]->SetSprite(_station_texture, tile);
                  _objects[snapshot->id]->EnableInterpolation();
                  _objects[snapshot->id]->SetPivot(glm::vec2(0.5f, 0.5f));
                } break;
              }
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

  bool _Loop() {   
    if (_network_state == NETWORK_STATE_LOGGED_IN) {
      uint32_t current_time = _GetTime();
      uint32_t delta_time = current_time - _last_loop;
      _last_loop = current_time;

      glm::float_t delta_x = (_keyboard_state.right - _keyboard_state.left) * _client_options->speed * delta_time;
      glm::float_t delta_y = (_keyboard_state.down - _keyboard_state.up) * _client_options->speed * delta_time;

      bool intersection_x = false;
      bool intersection_y = false;

      std::map<int, Object*>::iterator it;
      glm::vec2 player_position = _player->GetPosition();

      for(it = _walls.begin() ; it != _walls.end(); it++) {
        glm::vec2 wall_position = it->second->GetPosition();
        if(abs(wall_position.x - (player_position.x + delta_x)) < (_player_size + _wall_size) / 2 
          && abs(wall_position.y - (player_position.y)) < (_player_size + _wall_size) / 2) {
          intersection_x = true;
          break;
        }
      }
      for(it = _walls.begin() ; it != _walls.end(); it++) {
        glm::vec2 wall_position = it->second->GetPosition();
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

  bool _SendInputEvents() {
    std::vector<char> message;

    for(size_t i = 0; i < _keyboard_events.size(); i++) {
      message.clear();
      Packet::Type message_type = Packet::TYPE_KEYBOARD_EVENT;
      KeyboardEvent* event = &_keyboard_events[i];
      message.insert(message.end(), reinterpret_cast<char*>(&message_type),
        reinterpret_cast<char*>(&message_type) + sizeof(message_type));
      message.insert(message.end(), reinterpret_cast<char*>(event),
        reinterpret_cast<char*>(event) + sizeof(*event));
      bool rv = _peer->Send(&message[0], message.size());
      if(rv == false) {
        return false;
      }
    }
    _keyboard_events.clear();
    
    for(size_t i = 0; i < _mouse_events.size(); i++) {
      message.clear();
      Packet::Type message_type = Packet::TYPE_MOUSE_EVENT;
      MouseEvent* event = &_mouse_events[i];
      message.insert(message.end(), reinterpret_cast<char*>(&message_type),
        reinterpret_cast<char*>(&message_type) + sizeof(message_type));
      message.insert(message.end(), reinterpret_cast<char*>(event),
        reinterpret_cast<char*>(event) + sizeof(*event));
      bool rv = _peer->Send(&message[0], message.size());
      if(rv == false) {
        return false;
      }
    }
    _mouse_events.clear();

    return true;
  }

  // Returns approximate server time (with the correction).
  uint32_t _GetTime() {
    return SDL_GetTicks() + _time_correction;
  }
  
  void _RenderHUD() {
    _canvas.SetCoordinateType(Canvas::PixelsFlipped);
    _canvas.FillRect(glm::vec4(1, 0, 1, 0.8), glm::vec2(50, 90), glm::vec2(100 * _player_health / _client_options->max_health, 10));
    _canvas.FillRect(glm::vec4(0, 1, 1, 0.8), glm::vec2(50, 70), glm::vec2(100 * _player_blow_charge / _client_options->blow_capacity, 10));
    _canvas.FillRect(glm::vec4(1, 1, 0, 0.8), glm::vec2(50, 50), glm::vec2(100 * _player_morph_charge / _client_options->morph_capacity, 10));
  
    _canvas.SetCoordinateType(Canvas::Pixels);
    _canvas.DrawCircle(glm::vec4(1, 1, 1, 1), glm::vec2(80, 80), 60, 20);
    
    std::map<int, Object*>::iterator it;
    for (it = _objects.begin(); it != _objects.end(); ++it) {
      Object* obj = it->second;
      
      glm::vec2 rel = obj->GetPosition() - _player->GetPosition();
      if (glm::length(rel) < 400) {
        rel = rel * (60.0f / 400.0f);
        _canvas.DrawCircle(glm::vec4(1, 0, 0, 1), glm::vec2(80, 80) + rel, 1, 6);
      }
    }
  }

  // Draws all the objects.
  void _Render() {
    glClear(GL_COLOR_BUFFER_BIT);
    
    if (_network_state == NETWORK_STATE_LOGGED_IN) {  
      _canvas.SetCoordinateType(Canvas::Pixels);
      
      glMatrixMode(GL_MODELVIEW);
      glLoadIdentity();
      glTranslatef(_resolution_x / 2 - _player->GetPosition().x, _resolution_y / 2 - _player->GetPosition().y, 0);

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
        it->second->Render(_GetTime());
      }
      for(it = _objects.begin() ; it != _objects.end(); ++it) {
        it->second->Render(_GetTime());
      }

      _player->Render(_GetTime());
      
      _RenderHUD();

      _render_window.SwapBuffers();
    }
  }

  bool _Disconnect() {
    _peer->Disconnect();

    uint32_t start = _GetTime();
    while(_GetTime() - start <= _connect_timeout) {
      bool rv = _client->Service(_event, _connect_timeout);
      if(rv == false) {
        return false;
      }
      if(_event != NULL && _event->GetType() == Event::EVENT_DISCONNECT) {
        printf("Client disconnected.\n");
        return true;
      }
    }

    BM_ERROR("Not received EVENT_DISCONNECT event while disconnecting.\n");
    return false;
  }

  bool _is_running;

  Enet _enet;
  ClientHost* _client;
  Peer* _peer;
  Event* _event;

  // In milliseconds.
  uint32_t _connect_timeout;
  uint32_t _time_correction;
  uint32_t _last_tick;
  uint32_t _tick_rate;
  uint32_t _last_loop;

  int _resolution_x;
  int _resolution_y;

  RenderWindow _render_window;
  Canvas _canvas;

  // XXX[24.7.2012 alex]: copypasta
  TextureAtlas* _player_texture;
  TextureAtlas* _bullet_texture;
  TextureAtlas* _wall_texture;
  TextureAtlas* _dummy_texture;
  TextureAtlas* _explosion_texture;
  TextureAtlas* _station_texture;

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
    while(true);
    return EXIT_FAILURE;
  }
  app.Finalize();
  //bm::leak_detector::PrintAllLeaks();
  return EXIT_SUCCESS;
}

/*
#include "network_controller.hpp"
#include "packet_processer.hpp"

int main(int argc, char** argv) {
  NetworkController nc;
  bool rv = nc.Initialize("127.0.0.1", 4242);
  CHECK(rv == true);
  rv = nc.Connect(500);
  CHECK(rv == true);

  PacketProcesser pp;
  rv = pp.Initialize(&nc, (GameController*)(1));
  CHECK(rv == true);

  while(true) {
    rv = nc.Service();
    CHECK(rv == true);
  }

  return EXIT_SUCCESS;
}
*/
