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
#include <base/ini_file.hpp>

#include <enet-wrapper/enet.hpp>

#include <interpolator/interpolator.hpp>

#include "animation.hpp"
#include "render_window.hpp"
#include "sprite.hpp"
#include "texture_manager.hpp"
#include "text_writer.hpp"
#include "canvas.hpp"

using namespace bm;
using namespace protocol;

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

typedef interpolator::LinearInterpolator<ObjectState, bm::uint32_t> ObjectInterpolator;

// TODO[24.7.2012 alex]: fix method names
class Object {
public:
  Object(const glm::vec2& position, bm::uint32_t time, int id)
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

  bool SetSprite(Texture* texture, size_t tile = 0) {
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

  void UpdateCurrentState(const ObjectState& state, bm::uint32_t time) {
    if(_interpolation_enabled) {
      _interpolator.Push(state, time);
    }
    _current_state = state;
  }
  
  void EnforceState(const ObjectState& state, bm::uint32_t time) {
    _interpolator.Clear();
    _interpolator.Push(state, time);
    _current_state = state;
  }

  void Render(bm::uint32_t time) {
    if(_interpolation_enabled) {
      _current_state = _interpolator.Interpolate(time);
    }

    if(_sprite_set) {
      _sprite.SetPosition(_current_state.position);
      _sprite.Render();
    }

    if(_caption_enabled) {
      glm::vec2 caption_position = _current_state.position + _caption_pivot;
      _text_writer.PrintText(glm::vec4(1, 1, 1, 1), caption_position.x, caption_position.y, "%u\n(%f,%f)", _id,_current_state.position.x, _current_state.position.y);
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
  // XXX[24.7.2012 alex]: shouldn't we initialize object in the Init method?
  Application() {  
    _is_running = false;

    _resolution_x = 600;
    _resolution_y = 600;

    // TODO: think about more accurate name.
    _max_error = 25.f;

    _manager = NULL;
    _player_texture = NULL;
    _player = NULL;
    //_background = NULL;
    
    _client_options = NULL;
    _client_speed = 0.0f;

    _client = NULL;
    _peer = NULL;
    _event = NULL;
    
    _connect_timeout = 500;
    _last_tick = 0;
    _tickrate = 30;
    _time_correction = 0;

    _wall_size = 16;
    _player_size = 30;
    _state = STATE_FINALIZED;
  }

  ~Application() {
    CHECK(_state == STATE_FINALIZED);
  }
  
  void Init() {
    _state = STATE_INITIALIZED;
  }
  
  void Finalize() {
    if(_manager != NULL) {
      _manager->UnloadAll();
      delete _manager;
    }

    if(_player != NULL) delete _player;
    //if(_background != NULL) delete _background;
    if(_client_options != NULL) delete _client_options;

    if(_client != NULL) delete _client;
    if(_peer != NULL) delete _peer;
    if(_event != NULL) delete _event;

    std::map<int, Object*>::iterator it;
    for(it = _objects.begin(); it != _objects.end(); ++it) {
      delete it->second;
    }

    std::list<Animation*>::iterator it2;
    for(it2 = _animations.begin(); it2 != _animations.end(); ++it2) {
      delete *it2;
    }

    _render_window.Finalize();
    
    _state = STATE_FINALIZED;
  }

  bool Execute() {
    if (!bm::ini::LoadINI("data/client.ini", settings)) {
      return false;
    }
  
    // XXX[24.7.2012 alex]: switch to a FSM
    if(!_Initialize()) {
      return false;
    }

    /*if(!_ReceiveOptions()) {
      return false;
    }
    if(!_SynchronizeTime()) {
      return false;
    }*/

    /*_background = new Object(glm::vec2(0, 0), 4);
    if(_background == NULL) {
      // TODO: set error.
      return false;
    }
    _background->SetSprite(_wallpaper_texture);
    _background->SetPosition(glm::vec2(0, 0));*/

    // XXX[24.7.2012 alex]: hack
    _network_state = NETWORK_STATE_CONNECTED;
    _is_running = true;

    while(_is_running) {
      _PumpEvents();
      _Loop();
      _Render();
    }

    return true;
  }

private:
  bool _Initialize() {
    if (!_render_window.Init("Blowmorph", 600, 600, false)) {
      return false;
    }
    
    if (!_canvas.Init()) {
      return false;
    }

    if(!_InitializeTextures()) {
      return false;
    }

    if(!_InitializeNetwork()) {
      return false;
    }

    _keyboard_state.down = false;
    _keyboard_state.up = false;
    _keyboard_state.left = false;
    _keyboard_state.right = false;

    _last_loop = _GetTime();

    return true;
  }

  bool _InitializeTextures() {
    //FreeImage_Initialise();
    _manager = new TextureManager();
    if(_manager == NULL) {
      return false;
    }
    
    // XXX[24.7.2012 alex]: awful lot of copypasta
    
    //_player_texture = _manager->Load("data/images/ufo.png", (8 << 16) + (54 << 8) + 129);
    _player_texture = _manager->Load("data/images/player.png", 0);
    if(_player_texture == NULL) {
      return false;
    }

    _bullet_texture = _manager->Load("data/images/bullet.png", (8 << 16) + (54 << 8) + 129);
    if(_bullet_texture == NULL) {
      return false;
    }

    _wall_texture = _manager->Load("data/images/walls.png", 0, 1, 1, 17, 17, 16, 16);
    if(_wall_texture == NULL) {
      return false;
    }

    _dummy_texture = _manager->Load("data/images/guy.png", 0);
    if(_dummy_texture == NULL) {
      return false;
    }

    _explosion_texture = _manager->Load("data/images/explosion.png", 0, 1, 1, 61, 61, 60, 60);
    if(_explosion_texture == NULL) {
      return false;
    }

    /*_wallpaper_texture = _manager->Load("data/images/wallpaper4.jpg", (8 << 16) + (54 << 8) + 129);
    if(_wallpaper_texture == NULL) {
      return false;
    }*/

    return true;
  }

  bool _InitializeNetwork() {
    std::auto_ptr<ClientHost> client(_enet.CreateClientHost());
    if(client.get() == NULL) {
      return false;
    }

    std::string host = bm::ini::GetValue<std::string>(settings, "server.host", "127.0.0.1");
    bm::uint16_t port = bm::ini::GetValue(settings, "server.port", 4242);

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

    printf("Connected to %s:%u.\n", _event->GetPeerIp().c_str(), _event->GetPeerPort());

    return true;
  }

  void _PumpEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      _ProcessEvent(&event);
    }
  }

  void _ProcessEvent(SDL_Event* event) {
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
            _Disconnect();
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
            _Disconnect();
            break;
        }
        break;
      }

      case SDL_QUIT: {
        _Disconnect();
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
  }

  void Warning(const char* fmt, ...) {
    char buf[1024];
    
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf) - 1, fmt, args);
    va_end(args);
    
    fprintf(stderr, "WARN: %s\n", buf);
  }

  bool PumpPackets(bm::uint32_t timeout) {
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

          const PacketType* type = reinterpret_cast<PacketType*>(&message[0]);
          const void* data = reinterpret_cast<void*>(&message[0] + sizeof(PacketType)); 
          size_t len = message.size() - sizeof(PacketType);
          
          //printf("Received %d.", *type);
          
          // XXX[24.7.2012 alex]: will only process single packet at a time
          ProcessPacket(*type, data, len);
        } break;
        
        case Event::EVENT_CONNECT: {
          if (_network_state == NETWORK_STATE_CONNECTING) {
            _network_state = NETWORK_STATE_CONNECTED;
          } else {
            Warning("Got EVENT_CONNECT while being not in STATE_CONNECTING.");
          }
        } break;

        case Event::EVENT_DISCONNECT: {
          _network_state = NETWORK_STATE_DISCONNECTED;
          _is_running = false;
          // TODO: set error.
          return false;
        } break;
      }
    } while (_event->GetType() != Event::EVENT_NONE);
    
    return true;
  }

  bool ProcessPacket(PacketType type, const void* data, size_t len) {
    switch (_network_state) {
      case NETWORK_STATE_DISCONNECTED: {
        Warning("Received a packet while being in disconnected state.");
        return true;
      } break;
      case NETWORK_STATE_CONNECTING: {
        Warning("Received a packet while being in connecting state.");
        return true;
      } break;
      case NETWORK_STATE_CONNECTED: {
        if (type != BM_PACKET_CLIENT_OPTIONS) {
          Warning("Received packet with a type %d while waiting for client options.", type);
        } else if (len != sizeof(ClientOptions)) {
          Warning("Received packet has incorrect length.");
        } else {
          const ClientOptions* options = reinterpret_cast<const ClientOptions*>(data); 
          _client_options = new ClientOptions(*options);
          
          // switch to a new state
          _network_state = NETWORK_STATE_SYNCHRONIZATION;
          
          // send a time synchronization request
          TimeSyncData request_data;
          // XXX[24.7.2012 alex]: _GetTime()?
          request_data.client_time = SDL_GetTicks();
          PacketType request_type = BM_PACKET_SYNC_TIME_REQUEST;

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
        if (type != BM_PACKET_SYNC_TIME_RESPONSE) {
          Warning("Received packet with a type %d while waiting for time sync response.", type);
        } else if (len != sizeof(TimeSyncData)) {
          Warning("Received packet has incorrect length.");
        } else {
          const TimeSyncData* response_data = reinterpret_cast<const TimeSyncData*>(data);
          // Calculate the time correction.
          _time_correction = (SDL_GetTicks() - response_data->client_time) / 2 + response_data->server_time - SDL_GetTicks();

          printf("Time correction is %u ms.\n", _time_correction);

          bm::uint32_t latency = SDL_GetTicks() - response_data->client_time;
          printf("Latency is %u.\n", latency);

          _network_state = NETWORK_STATE_LOGGED_IN;
          
          // XXX[24.7.2012 alex]: move it to the ProcessPacket method
          _player = new Object(glm::vec2(_client_options->x, _client_options->y), 0, _client_options->id);
          if(_player == NULL) {
            // TODO: set error.
            return false;
          }
          // XXX[24.7.2012 alex]: maybe we should have a xml file for each object with
          //                      texture paths, pivots, captions, etc
          _player->SetSprite(_player_texture);
          _player->SetPosition(glm::vec2(_client_options->x, _client_options->y));
          _player->SetPivot(glm::vec2(0.5f, 0.5f));
          _player->EnableCaption();

          // XXX[24.7.2012 alex]: why make a new variable?
          _client_speed = _client_options->speed;
        }
        
        return true;
      } break;
      case NETWORK_STATE_LOGGED_IN: {
        bool isSnapshot = len == sizeof(EntitySnapshot) &&
                          (type == BM_PACKET_ENTITY_APPEARED ||
                           type == BM_PACKET_ENTITY_DISAPPEARED ||
                           type == BM_PACKET_ENTITY_UPDATED);
                           
        if (isSnapshot) {
          const EntitySnapshot* snapshot = reinterpret_cast<const EntitySnapshot*>(data);
          if (type == BM_PACKET_ENTITY_DISAPPEARED) {
            _objects.erase(snapshot->id);
            _walls.erase(snapshot->id);

            if(snapshot->type == BM_ENTITY_BULLET) {
              // TODO[12.08.2012 xairy]: create explosion animation on explosion packet.
              // TODO[12.08.2012 xairy]: remove magic numbers;
              Animation* animation = new Animation();
              if(animation == NULL) {
                Error::Set(Error::TYPE_MEMORY);
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
            bm::uint32_t time = snapshot->time;

            if(snapshot->id == _player->GetId()) {
              // TODO: fix it after changing protocol.
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
              // TODO: state constructor.
              ObjectState state;
              state.position = position;
              state.health = static_cast<float>(snapshot->data[0]);
              state.blowCharge = static_cast<float>(snapshot->data[1]);
              state.morphCharge = static_cast<float>(snapshot->data[2]);
              
              if(snapshot->type == BM_ENTITY_WALL) {
                _walls[snapshot->id]->UpdateCurrentState(state, time);
              } else {
                _objects[snapshot->id]->UpdateCurrentState(state, time);
              }
            } else {
              switch(snapshot->type) {
                case BM_ENTITY_WALL: {
                  _walls[snapshot->id] = new Object(position, time, snapshot->id);
                  size_t tile;
                  if(snapshot->data[0] == BM_WALL_ORDINARY) {
                    tile = 3;
                  } else if(snapshot->data[0] == BM_WALL_UNBREAKABLE) {
                    tile = 2;
                  } else if(snapshot->data[0] == BM_WALL_MORPHED) {
                    tile = 1;
                  }
                  _walls[snapshot->id]->SetSprite(_wall_texture, tile);
                  _walls[snapshot->id]->EnableInterpolation();
                  _walls[snapshot->id]->SetPivot(glm::vec2(0.5f, 0.5f));
                } break;

                case BM_ENTITY_BULLET: {
                  _objects[snapshot->id] = new Object(position, time, snapshot->id);
                  _objects[snapshot->id]->SetSprite(_bullet_texture);
                  _objects[snapshot->id]->EnableInterpolation();
                  _objects[snapshot->id]->SetPivot(glm::vec2(0.5f, 0.5f));
                } break;

                case BM_ENTITY_PLAYER: {
                  _objects[snapshot->id] = new Object(position, time, snapshot->id);
                  _objects[snapshot->id]->SetSprite(_player_texture);
                  _objects[snapshot->id]->EnableInterpolation();
                  _objects[snapshot->id]->SetPivot(glm::vec2(0.5f, 0.5f));
                  _objects[snapshot->id]->EnableCaption();
                } break;

                case BM_ENTITY_DUMMY: {
                  _objects[snapshot->id] = new Object(position, time, snapshot->id);
                  _objects[snapshot->id]->SetSprite(_dummy_texture);
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
    // XXX[24.7.2012 alex]: method is too long
    
    if (_network_state == NETWORK_STATE_LOGGED_IN) {
      bm::uint32_t current_time = _GetTime();
      bm::uint32_t delta_time = current_time - _last_loop;
      _last_loop = current_time;

      glm::float_t delta_x = (_keyboard_state.right - _keyboard_state.left) * _client_speed * delta_time;
      glm::float_t delta_y = (_keyboard_state.down - _keyboard_state.up) * _client_speed * delta_time;

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

      if(current_time - _last_tick > 1000.0 / _tickrate) {
        _last_tick = current_time;
        _SendInputEvents();
      }
    }
    
    PumpPackets(0);
    
    return true;
  }

  bool _SendInputEvents() {
    std::vector<char> message;

    for(size_t i = 0; i < _keyboard_events.size(); i++) {
      message.clear();
      PacketType message_type = BM_PACKET_KEYBOARD_EVENT;
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
      PacketType message_type = BM_PACKET_MOUSE_EVENT;
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
  bm::uint32_t _GetTime() {
    return SDL_GetTicks() + _time_correction;
  }
  
  void _RenderHUD() {
    _canvas.SetCoordinateType(Canvas::PixelsFlipped);
    _canvas.FillRect(glm::vec4(1, 1, 0, 0.8), glm::vec2(50, 50), glm::vec2(100 * _player_health / _client_options->max_health, 10));
    _canvas.FillRect(glm::vec4(0, 1, 1, 0.8), glm::vec2(50, 70), glm::vec2(100 * _player_blow_charge / _client_options->blow_capacity, 10));
    _canvas.FillRect(glm::vec4(1, 0, 1, 0.8), glm::vec2(50, 90), glm::vec2(100 * _player_morph_charge / _client_options->morph_capacity, 10));
  }

  // Draws all the objects.
  void _Render() {
    glClear(GL_COLOR_BUFFER_BIT);
    
    if (_network_state == NETWORK_STATE_LOGGED_IN) {  
      _canvas.SetCoordinateType(Canvas::Pixels);
      
      glMatrixMode(GL_MODELVIEW);
      glLoadIdentity();
      glTranslatef(_resolution_x / 2 - _player->GetPosition().x, _resolution_y / 2 - _player->GetPosition().y, 0);
      
      //_background->Render(_GetTime());

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

    // TODO: make some attempts?
    while(true) {
      bool rv = _client->Service(_event, _connect_timeout);
      if(rv == false) {
        return false;
      }
      if(_event != NULL && _event->GetType() == Event::EVENT_DISCONNECT) {
        break;
      }
    }

    printf("Client disconnected.\n");
    return true;
  }

  RenderWindow _render_window;
  Canvas _canvas;

  float _player_health;
  float _player_blow_charge;
  float _player_morph_charge;

  bool _is_running;

  int _resolution_x;
  int _resolution_y;

  glm::float_t _max_error;

  TextureManager* _manager;
  // XXX[24.7.2012 alex]: copypasta
  Texture* _player_texture;
  Texture* _bullet_texture;
  Texture* _wall_texture;
  Texture* _dummy_texture;
  Texture* _explosion_texture;
  //Texture* _wallpaper_texture;
  Object* _player;
  //Object* _background;

  ClientOptions* _client_options;
  float _client_speed;

  Enet _enet;
  ClientHost* _client;
  Peer* _peer;
  Event* _event;

  // In milliseconds.
  bm::uint32_t _connect_timeout;
  bm::uint32_t _last_tick;
  bm::uint32_t _tickrate;
  bm::uint32_t _time_correction;
  bm::uint32_t _last_loop;

  bm::uint32_t _wall_size;
  bm::uint32_t _player_size;

  std::map<int, Object*> _objects;
  std::map<int, Object*> _walls;

  std::list<Animation*> _animations;

  // Input events since the last tick.
  std::vector<KeyboardEvent> _keyboard_events;
  std::vector<MouseEvent> _mouse_events;

  struct KeyboardState {
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
    NETWORK_STATE_CONNECTING,
    NETWORK_STATE_CONNECTED,
    NETWORK_STATE_SYNCHRONIZATION,
    NETWORK_STATE_LOGGED_IN
  };
  NetworkState _network_state;
  
  bm::ini::RecordMap settings;
};

int main(int argc, char** argv) { 
  Application app;
  app.Init();
  if(!app.Execute()) {
    Error::Print();
    app.Finalize();
    return EXIT_FAILURE;
  }
  app.Finalize();
  return EXIT_SUCCESS;
}
