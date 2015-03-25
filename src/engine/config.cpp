// Copyright (c) 2015 Blowmorph Team

#include "engine/config.h"

#include <map>
#include <string>
#include <fstream>  // NOLINT
#include <vector>

#include "base/error.h"
#include "base/json.h"
#include "base/macros.h"
#include "base/pstdint.h"
#include "base/singleton.h"

namespace bm {

Config::Config() : state_(STATE_FINALIZED) { }
Config::~Config() { }

bool Config::Initialize() {
  CHECK(state_ == STATE_FINALIZED);
  if (!LoadMasterServerConfig() || !LoadServerConfig() || !LoadClientConfig()) {
    return false;
  }
  if (!LoadBodiesConfig() || !LoadTexturesConfig() || !LoadSpritesConfig()) {
    return false;
  }
  if (!LoadActivatorsConfig() || !LoadCrittersConfig() || !LoadDoorsConfig() ||
      !LoadKitsConfig() || !LoadPlayersConfig() || !LoadProjectilesConfig() ||
      !LoadWallsConfig()) {
    return false;
  }
  if (!LoadGunsConfig()) {
    return false;
  }
  state_ = STATE_INITIALIZED;
  return true;
}

const Config::MasterServerConfig& Config::GetMasterServerConfig() const {
  CHECK(state_ == STATE_INITIALIZED);
  return master_server_;
}

const Config::ServerConfig& Config::GetServerConfig() const {
  CHECK(state_ == STATE_INITIALIZED);
  return server_;
}

const Config::ClientConfig& Config::GetClientConfig() const {
  CHECK(state_ == STATE_INITIALIZED);
  return client_;
}

const std::map<std::string, Config::BodyConfig>&
Config::GetBodiesConfig() const {
  CHECK(state_ == STATE_INITIALIZED);
  return bodies_;
}

const std::map<std::string, Config::TextureConfig>&
Config::GetTexturesConfig() const {
  CHECK(state_ == STATE_INITIALIZED);
  return textures_;
}

const std::map<std::string, Config::SpriteConfig>&
Config::GetSpritesConfig() const {
  CHECK(state_ == STATE_INITIALIZED);
  return sprites_;
}

const std::map<std::string, Config::ActivatorConfig>&
Config::GetActivatorsConfig() const {
  CHECK(state_ == STATE_INITIALIZED);
  return activators_;
}

const std::map<std::string, Config::CritterConfig>&
Config::GetCrittersConfig() const {
  CHECK(state_ == STATE_INITIALIZED);
  return critters_;
}

const std::map<std::string, Config::DoorConfig>&
Config::GetDoorsConfig() const {
  CHECK(state_ == STATE_INITIALIZED);
  return doors_;
}

const std::map<std::string, Config::KitConfig>&
Config::GetKitsConfig() const {
  CHECK(state_ == STATE_INITIALIZED);
  return kits_;
}

const std::map<std::string, Config::PlayerConfig>&
Config::GetPlayersConfig() const {
  CHECK(state_ == STATE_INITIALIZED);
  return players_;
}

const std::map<std::string, Config::ProjectileConfig>&
Config::GetProjectilesConfig() const {
  CHECK(state_ == STATE_INITIALIZED);
  return projectiles_;
}

const std::map<std::string, Config::WallConfig>&
Config::GetWallsConfig() const {
  CHECK(state_ == STATE_INITIALIZED);
  return walls_;
}

const std::map<std::string, Config::GunConfig>&
Config::GetGunsConfig() const {
  CHECK(state_ == STATE_INITIALIZED);
  return guns_;
}

bool Config::LoadMasterServerConfig() {
  std::string file = "data/master-server.json";
  Json::Reader reader;
  Json::Value root;

  if (!ParseFile(file, &reader, &root)) {
      REPORT_ERROR("Can't parse file '%s'.", file.c_str());
      return false;
  }

  Json::Value master_server = root["master-server"];
  if (master_server == Json::Value::null || !master_server.isObject()) {
    REPORT_ERROR("Config '%s' of type '%s' not found in '%s'.",
        "master-server", "object", file.c_str());
    return false;
  }

  // FIXME(xairy): use GetUInt16.
  uint32_t port;
  if (!GetUInt32(master_server["port"], &port)) {
    REPORT_ERROR("Config '%s.%s' of type '%s' not found in '%s'.",
        "master_server", "port", "int", file.c_str());
    return false;
  }
  master_server_.port = static_cast<uint16_t>(port);

  return true;
}

bool Config::LoadServerConfig() {
  std::string file = "data/server.json";
  Json::Reader reader;
  Json::Value root;

  if (!ParseFile(file, &reader, &root)) {
      REPORT_ERROR("Can't parse file '%s'.", file.c_str());
      return false;
  }

  Json::Value server = root["server"];
  if (server == Json::Value::null || !server.isObject()) {
    REPORT_ERROR("Config '%s' of type '%s' not found in '%s'.",
        "server", "object", file.c_str());
    return false;
  }
  // FIXME(xairy): use GetUInt16.
  uint32_t port;
  if (!GetUInt32(server["port"], &port)) {
    REPORT_ERROR("Config '%s.%s' of type '%s' not found in '%s'.",
        "server", "port", "int", file.c_str());
    return false;
  }
  server_.port = static_cast<uint16_t>(port);
  if (!GetInt32(server["broadcast_rate"], &server_.broadcast_rate)) {
    REPORT_ERROR("Config '%s.%s' of type '%s' not found in '%s'.",
        "server", "broadcast_rate", "int", file.c_str());
    return false;
  }
  if (!GetInt32(server["tick_rate"], &server_.tick_rate)) {
    REPORT_ERROR("Config '%s.%s' of type '%s' not found in '%s'.",
        "server", "tick_rate", "int", file.c_str());
    return false;
  }
  if (!GetString(server["map"], &server_.map)) {
    REPORT_ERROR("Config '%s.%s' of type '%s' not found in '%s'.",
        "server", "map", "string", file.c_str());
    return false;
  }
  if (!GetString(server["name"], &server_.name)) {
    REPORT_ERROR("Config '%s.%s' of type '%s' not found in '%s'.",
        "server", "name", "string", file.c_str());
    return false;
  }

  Json::Value master_server = root["master-server"];
  if (master_server == Json::Value::null || !master_server.isObject()) {
    REPORT_ERROR("Config '%s' of type '%s' not found in '%s'.",
        "master-server", "object", file.c_str());
    return false;
  }
  if (!GetString(master_server["host"], &server_.master_server_host)) {
    REPORT_ERROR("Config '%s.%s' of type '%s' not found in '%s'.",
        "master-server", "host", "string", file.c_str());
    return false;
  }
  // FIXME(xairy): use GetUInt16.
  if (!GetUInt32(master_server["port"], &port)) {
    REPORT_ERROR("Config '%s.%s' of type '%s' not found in '%s'.",
        "master-server", "port", "int", file.c_str());
    return false;
  }
  server_.master_server_port = static_cast<uint16_t>(port);

  return true;
}

bool Config::LoadClientConfig() {
  std::string file = "data/client.json";
  Json::Reader reader;
  Json::Value root;

  if (!ParseFile(file, &reader, &root)) {
      REPORT_ERROR("Can't parse file '%s'.", file.c_str());
      return false;
  }

  Json::Value server = root["server"];
  if (server == Json::Value::null || !server.isObject()) {
    REPORT_ERROR("Config '%s' of type '%s' not found in '%s'.",
        "server", "object", file.c_str());
    return false;
  }
  if (!GetString(server["host"], &client_.server_host)) {
    REPORT_ERROR("Config '%s.%s' of type '%s' not found in '%s'.",
        "server", "host", "string", file.c_str());
    return false;
  }
  // FIXME(xairy): use GetUInt16.
  uint32_t port;
  if (!GetUInt32(server["port"], &port)) {
    REPORT_ERROR("Config '%s.%s' of type '%s' not found in '%s'.",
        "server", "port", "int", file.c_str());
    return false;
  }
  client_.server_port = static_cast<uint16_t>(port);

  Json::Value master_server = root["master-server"];
  if (master_server == Json::Value::null || !master_server.isObject()) {
    REPORT_ERROR("Config '%s' of type '%s' not found in '%s'.",
        "master-server", "object", file.c_str());
    return false;
  }
  if (!GetString(master_server["host"], &client_.master_server_host)) {
    REPORT_ERROR("Config '%s.%s' of type '%s' not found in '%s'.",
        "master-server", "host", "string", file.c_str());
    return false;
  }
  // FIXME(xairy): use GetUInt16.
  if (!GetUInt32(master_server["port"], &port)) {
    REPORT_ERROR("Config '%s.%s' of type '%s' not found in '%s'.",
        "master-server", "port", "int", file.c_str());
    return false;
  }
  client_.master_server_port = static_cast<uint16_t>(port);

  Json::Value player = root["player"];
  if (player == Json::Value::null || !player.isObject()) {
    REPORT_ERROR("Config '%s' of type '%s' not found in '%s'.",
        "player", "object", file.c_str());
    return false;
  }
  if (!GetString(player["name"], &client_.player_name)) {
    REPORT_ERROR("Config '%s.%s' of type '%s' not found in '%s'.",
        "player", "name", "string", file.c_str());
    return false;
  }

  Json::Value graphics = root["graphics"];
  if (graphics == Json::Value::null || !graphics.isObject()) {
    REPORT_ERROR("Config '%s' of type '%s' not found in '%s'.",
        "graphics", "object", file.c_str());
    return false;
  }
  if (!GetInt32(graphics["width"], &client_.screen_width)) {
    REPORT_ERROR("Config '%s.%s' of type '%s' not found in '%s'.",
        "graphics", "width", "int", file.c_str());
    return false;
  }
  if (!GetInt32(graphics["height"], &client_.screen_height)) {
    REPORT_ERROR("Config '%s.%s' of type '%s' not found in '%s'.",
        "graphics", "height", "int", file.c_str());
    return false;
  }
  if (!GetBool(graphics["fullscreen"], &client_.fullscreen)) {
    REPORT_ERROR("Config '%s.%s' of type '%s' not found in '%s'.",
        "graphics", "fullscreen", "bool", file.c_str());
    return false;
  }

  Json::Value net = root["net"];
  if (net == Json::Value::null || !net.isObject()) {
    REPORT_ERROR("Config '%s' of type '%s' not found in '%s'.",
        "net", "object", file.c_str());
    return false;
  }
  if (!GetInt32(net["tick_rate"], &client_.tick_rate)) {
    REPORT_ERROR("Config '%s.%s' of type '%s' not found in '%s'.",
        "net", "tick_rate", "int", file.c_str());
    return false;
  }
  if (!GetInt32(net["connect_timeout"], &client_.connect_timeout)) {
    REPORT_ERROR("Config '%s.%s' of type '%s' not found in '%s'.",
        "net", "connect_timeout", "int", file.c_str());
    return false;
  }
  if (!GetInt32(net["sync_timeout"], &client_.sync_timeout)) {
    REPORT_ERROR("Config '%s.%s' of type '%s' not found in '%s'.",
        "net", "sync_timeout", "int", file.c_str());
    return false;
  }
  if (!GetFloat32(net["max_player_misposition"],
                &client_.max_player_misposition)) {
    REPORT_ERROR("Config '%s.%s' of type '%s' not found in '%s'.",
        "net", "max_player_misposition", "float", file.c_str());
    return false;
  }
  if (!GetInt32(net["interpolation_offset"], &client_.interpolation_offset)) {
    REPORT_ERROR("Config '%s.%s' of type '%s' not found in '%s'.",
        "net", "interpolation_offset", "int", file.c_str());
    return false;
  }

  return true;
}

bool Config::LoadBodiesConfig() {
  std::string file = "data/bodies.json";
  Json::Reader reader;
  Json::Value root;

  if (!ParseFile(file, &reader, &root)) {
      REPORT_ERROR("Can't parse file '%s'.", file.c_str());
      return false;
  }

  Json::Value bodies = root["bodies"];
  if (bodies == Json::Value::null || !bodies.isArray()) {
    REPORT_ERROR("Config '%s' of type '%s' not found in '%s'.",
        "bodies", "array", file.c_str());
    return false;
  }
  if (bodies.size() == 0) {
    REPORT_ERROR("Array '%s' is empty in '%s'.", "bodies", file.c_str());
    return false;
  }

  for (int i = 0; i < bodies.size(); i++) {
    std::string name;
    if (!GetString(bodies[i]["name"], &name)) {
      REPORT_ERROR("Config 'bodies[%d].%s' of type '%s' not found in '%s'.",
          i, "name", "string", file.c_str());
      return false;
    }
    if (bodies_.count(name) != 0) {
      REPORT_ERROR("Body '%s' defined twice in '%s'.",
          name.c_str(), file.c_str());
      return false;
    }
    bodies_[name].name = name;

    if (!GetBool(bodies[i]["dynamic"], &bodies_[name].dynamic)) {
      REPORT_ERROR("Config 'bodies[%d].%s' of type '%s' not found in '%s'.",
          i, "dynamic", "bool", file.c_str());
      return false;
    }

    Json::Value shape = bodies[i]["shape"];
    if (shape == Json::Value::null) {
      REPORT_ERROR("Config 'bodies[%d].%s' of type '%s' not found in '%s'.",
          i, "shape", "array", file.c_str());
      return false;
    }

    std::string type;
    if (!GetString(shape["type"], &type)) {
      REPORT_ERROR("Config 'bodies[%d][%s].%s' of type '%s' not found in '%s'.",
          i, "shape", "type", "string", file.c_str());
      return false;
    }
    if (type == "box") {
      bodies_[name].shape_type = Config::BodyConfig::SHAPE_TYPE_BOX;
      if (!GetFloat32(shape["width"], &bodies_[name].box_config.width)) {
        REPORT_ERROR("Config '%s[%d].%s..%s' of type '%s' not found in '%s'.",
            "bodies", i, "shape", "width", "float", file.c_str());
        return false;
      }
      if (!GetFloat32(shape["height"], &bodies_[name].box_config.height)) {
        REPORT_ERROR("Config '%s[%d].%s.%s' of type '%s' not found in '%s'.",
            "bodies", i, "shape", "height", "float", file.c_str());
        return false;
      }
    } else if (type == "circle") {
      bodies_[name].shape_type = Config::BodyConfig::SHAPE_TYPE_CIRCLE;
      if (!GetFloat32(shape["radius"], &bodies_[name].circle_config.radius)) {
        REPORT_ERROR("Config '%s[%d].%s.%s' of type '%s' not found in '%s'.",
            "bodies", i, "shape", "radius", "float", file.c_str());
        return false;
      }
    } else if (type == "polygon") {
      bodies_[name].shape_type = Config::BodyConfig::SHAPE_TYPE_POLYGON;
      Json::Value vertices = shape["vertices"];
      if (vertices == Json::Value::null || !vertices.isArray()) {
        REPORT_ERROR("Config '%s[%d].%s.%s' of type '%s' not found in '%s'.",
            "bodies", i, "shape", "vectices", "array", file.c_str());
        return false;
      }
      if (vertices.size() == 0) {
        REPORT_ERROR("Array '%s[%d].%s.%s' is empty in '%s'.",
          "bodies", i, "shape", "vectices", file.c_str());
        return false;
      }
      for (int v = 0; v < vertices.size(); v++) {
        float32_t x, y;
        if (!vertices[v].isObject()) {
          REPORT_ERROR("Array '%s[%d].%s.%s' must contain a list of objects.",
            "bodies", v, "shape", "vectices", file.c_str());
          return false;
        }
        if (!GetFloat32(vertices[v]["x"], &x)) {
          REPORT_ERROR(
              "Config '%s[%d].%s.%s[%d].x' of type '%s' not found in '%s'.",
              "bodies", i, "shape", "vertices", v, "float", file.c_str());
          return false;
        }
        if (!GetFloat32(vertices[v]["y"], &y)) {
          REPORT_ERROR(
              "Config '%s[%d].%s.%s[%d].y' of type '%s' not found in '%s'.",
              "bodies", i, "shape", "vertices", v, "float", file.c_str());
          return false;
        }
        bodies_[name].polygon_config.vertices.push_back(
          BodyConfig::PolygonConfig::Vertice {x, y});
      }
    } else {
      REPORT_ERROR("Config bodies[%d].%s.%s must be 'box' or 'circle' in %s.",
          i, "shape", "type", file.c_str());
      return false;
    }
  }

  return true;
}

bool Config::LoadTexturesConfig() {
  std::string file = "data/textures.json";
  Json::Reader reader;
  Json::Value root;

  if (!ParseFile(file, &reader, &root)) {
      REPORT_ERROR("Can't parse file '%s'.", file.c_str());
      return false;
  }

  Json::Value textures = root["textures"];
  if (textures == Json::Value::null || !textures.isArray()) {
    REPORT_ERROR("Config '%s' of type '%s' not found in '%s'.",
        "textures", "array", file.c_str());
    return false;
  }
  if (textures.size() == 0) {
    REPORT_ERROR("Array '%s' is empty in '%s'.", "textures", file.c_str());
    return false;
  }

  for (int i = 0; i < textures.size(); i++) {
    std::string name;
    if (!GetString(textures[i]["name"], &name)) {
      REPORT_ERROR("Config 'textures[%d].%s' of type '%s' not found in '%s'.",
          i, "name", "string", file.c_str());
      return false;
    }
    if (textures_.count(name) != 0) {
      REPORT_ERROR("Texture '%s' defined twice in '%s'.",
          name.c_str(), file.c_str());
      return false;
    }
    textures_[name].name = name;

    if (!GetString(textures[i]["image"], &textures_[name].image)) {
      REPORT_ERROR("Config 'textures[%d].%s' of type '%s' not found in '%s'.",
          i, "image", "string", file.c_str());
      return false;
    }
    if (!GetUInt32(textures[i]["transparent_color"],
                   &textures_[name].transparent_color)) {
      textures_[name].transparent_color = 0xFFFFFFFF;
    }

    Json::Value tile = textures[i]["tile"];
    if (tile == Json::Value::null) {
      textures_[name].tiled = false;
      continue;
    }
    if (!tile.isObject()) {
      REPORT_ERROR("Config '%s[%d].%s' of type '%s' not found in '%s'.",
          "textures", i, "tile", "object", file.c_str());
      return false;
    }
    textures_[name].tiled = true;

    if (!GetInt32(tile["start_x"], &textures_[name].tile_start_x)) {
      REPORT_ERROR("Config '%s[%d].%s.%s' of type '%s' not found in '%s'.",
          "textures", i, "tile", "start_x", "int", file.c_str());
      return false;
    }
    if (!GetInt32(tile["start_y"], &textures_[name].tile_start_y)) {
      REPORT_ERROR("Config '%s[%d].%s.%s' of type '%s' not found in '%s'.",
          "textures", i, "tile", "start_y", "int", file.c_str());
      return false;
    }
    if (!GetInt32(tile["step_x"], &textures_[name].tile_step_x)) {
      REPORT_ERROR("Config '%s[%d].%s.%s' of type '%s' not found in '%s'.",
          "textures", i, "tile", "step_x", "int", file.c_str());
      return false;
    }
    if (!GetInt32(tile["step_y"], &textures_[name].tile_step_y)) {
      REPORT_ERROR("Config '%s[%d].%s.%s' of type '%s' not found in '%s'.",
          "textures", i, "tile", "step_y", "int", file.c_str());
      return false;
    }
    if (!GetInt32(tile["width"], &textures_[name].tile_width)) {
      REPORT_ERROR("Config '%s[%d].%s.%s' of type '%s' not found in '%s'.",
          "textures", i, "tile", "width", "int", file.c_str());
      return false;
    }
    if (!GetInt32(tile["height"], &textures_[name].tile_height)) {
      REPORT_ERROR("Config '%s[%d].%s.%s' of type '%s' not found in '%s'.",
          "textures", i, "tile", "height", "int", file.c_str());
      return false;
    }
  }

  return true;
}

bool Config::LoadSpritesConfig() {
  std::string file = "data/sprites.json";
  Json::Reader reader;
  Json::Value root;

  if (!ParseFile(file, &reader, &root)) {
      REPORT_ERROR("Can't parse file '%s'.", file.c_str());
      return false;
  }

  Json::Value sprites = root["sprites"];
  if (sprites == Json::Value::null || !sprites.isArray()) {
    REPORT_ERROR("Config '%s' of type '%s' not found in '%s'.",
        "sprites", "array", file.c_str());
    return false;
  }
  if (sprites.size() == 0) {
    REPORT_ERROR("Array '%s' is empty in '%s'.", "sprites", file.c_str());
    return false;
  }

  for (int i = 0; i < sprites.size(); i++) {
    std::string name;
    if (!GetString(sprites[i]["name"], &name)) {
      REPORT_ERROR("Config 'sprites[%d].%s' of type '%s' not found in '%s'.",
          i, "name", "string", file.c_str());
      return false;
    }
    if (sprites_.count(name) != 0) {
      REPORT_ERROR("Sprite '%s' defined twice in '%s'.",
          name.c_str(), file.c_str());
      return false;
    }
    sprites_[name].name = name;

    if (!GetString(sprites[i]["texture"], &sprites_[name].texture_name)) {
      REPORT_ERROR("Config 'sprites[%d].%s' of type '%s' not found in '%s'.",
          i, "texture", "string", file.c_str());
      return false;
    }

    Json::Value mode = sprites[i]["mode"];
    if (mode == Json::Value::null) {
      sprites_[name].has_modes = false;
      sprites_[name].mode.timeout = 0;
      sprites_[name].mode.cyclic = false;
      continue;
    }
    if (!mode.isObject()) {
      REPORT_ERROR("Config '%s[%d].%s' of type '%s' not found in '%s'.",
          "sprites", i, "mode", "object", file.c_str());
      return false;
    }
    sprites_[name].has_modes = true;

    Json::Value tiles = mode["tiles"];
    if (tiles == Json::Value::null || !tiles.isArray()) {
      REPORT_ERROR("Config '%s[%d].%s.%s' of type '%s' not found in '%s'.",
          "sprites", i, "mode", "tiles", "array", file.c_str());
      return false;
    }
    for (int j = 0; j < tiles.size(); j++) {
      int tile;
      if (!GetInt32(tiles[j], &tile)) {
        REPORT_ERROR(
            "Config '%s[%d].%s.%s[%d]' of type '%s' not found in '%s'.",
            "textures", i, "mode", "tiles", j, "int", file.c_str());
        return false;
      }
      sprites_[name].mode.tiles.push_back(tile);
    }

    if (!GetInt32(mode["timeout"], &sprites_[name].mode.timeout)) {
      sprites_[name].mode.timeout = 0;
    }
    if (!GetBool(mode["cyclic"], &sprites_[name].mode.cyclic)) {
      sprites_[name].mode.cyclic = false;
    }
  }

  return true;
}

bool Config::LoadActivatorsConfig() {
  std::string file = "data/entities.json";
  Json::Reader reader;
  Json::Value root;

  if (!ParseFile(file, &reader, &root)) {
      REPORT_ERROR("Can't parse file '%s'.", file.c_str());
      return false;
  }

  Json::Value activators = root["activators"];
  if (activators == Json::Value::null || !activators.isArray()) {
    REPORT_ERROR("Config '%s' of type '%s' not found in '%s'.",
        "activators", "array", file.c_str());
    return false;
  }

  for (int i = 0; i < activators.size(); i++) {
    std::string name;
    if (!GetString(activators[i]["name"], &name)) {
      REPORT_ERROR("Config 'activators[%d].%s' of type '%s' not found in '%s'.",
          i, "name", "string", file.c_str());
      return false;
    }
    if (activators_.count(name) != 0) {
      REPORT_ERROR("Activator '%s' defined twice in '%s'.",
          name.c_str(), file.c_str());
      return false;
    }
    activators_[name].name = name;

    if (!GetString(activators[i]["body"], &activators_[name].body_name)) {
      REPORT_ERROR("Config 'activators[%d].%s' of type '%s' not found in '%s'.",
          i, "body", "string", file.c_str());
      return false;
    }
    if (!GetString(activators[i]["sprite"], &activators_[name].sprite_name)) {
      REPORT_ERROR("Config 'activators[%d].%s' of type '%s' not found in '%s'.",
          i, "sprite", "string", file.c_str());
      return false;
    }
    if (!GetFloat32(activators[i]["activation_distance"],
                    &activators_[name].activation_distance)) {
      REPORT_ERROR("Config 'activators[%d].%s' of type '%s' not found in '%s'.",
          i, "activation_distance", "float", file.c_str());
      return false;
    }
  }

  return true;
}

bool Config::LoadCrittersConfig() {
  std::string file = "data/entities.json";
  Json::Reader reader;
  Json::Value root;

  if (!ParseFile(file, &reader, &root)) {
      REPORT_ERROR("Can't parse file '%s'.", file.c_str());
      return false;
  }

  Json::Value critters = root["critters"];
  if (critters == Json::Value::null || !critters.isArray()) {
    REPORT_ERROR("Config '%s' of type '%s' not found in '%s'.",
        "critters", "array", file.c_str());
    return false;
  }

  for (int i = 0; i < critters.size(); i++) {
    std::string name;
    if (!GetString(critters[i]["name"], &name)) {
      REPORT_ERROR("Config 'critters[%d].%s' of type '%s' not found in '%s'.",
          i, "name", "string", file.c_str());
      return false;
    }
    if (critters_.count(name) != 0) {
      REPORT_ERROR("Critter '%s' defined twice in '%s'.",
          name.c_str(), file.c_str());
      return false;
    }
    critters_[name].name = name;

    if (!GetString(critters[i]["body"], &critters_[name].body_name)) {
      REPORT_ERROR("Config 'critters[%d].%s' of type '%s' not found in '%s'.",
          i, "body", "string", file.c_str());
      return false;
    }
    if (!GetString(critters[i]["sprite"], &critters_[name].sprite_name)) {
      REPORT_ERROR("Config 'critters[%d].%s' of type '%s' not found in '%s'.",
          i, "sprite", "string", file.c_str());
      return false;
    }
    if (!GetFloat32(critters[i]["speed"], &critters_[name].speed)) {
      REPORT_ERROR("Config 'critters[%d].%s' of type '%s' not found in '%s'.",
          i, "speed", "float", file.c_str());
      return false;
    }
    if (!GetInt32(critters[i]["damage"], &critters_[name].damage)) {
      REPORT_ERROR("Config 'critters[%d].%s' of type '%s' not found in '%s'.",
          i, "damage", "int", file.c_str());
      return false;
    }

    std::string type;
    if (!GetString(critters[i]["type"], &type)) {
      REPORT_ERROR("Config 'critters[%d].%s' of type '%s' not found in '%s'.",
          i, "type", "string", file.c_str());
      return false;
    }
    if (type == "zombie") {
      critters_[name].type = CritterConfig::TYPE_ZOMBIE;
    } else {
      REPORT_ERROR("Config critters[%d].%s must be 'zombie' in %s.",
          i, "type", file.c_str());
      return false;
    }
  }

  return true;
}

bool Config::LoadDoorsConfig() {
  std::string file = "data/entities.json";
  Json::Reader reader;
  Json::Value root;

  if (!ParseFile(file, &reader, &root)) {
      REPORT_ERROR("Can't parse file '%s'.", file.c_str());
      return false;
  }

  Json::Value doors = root["doors"];
  if (doors == Json::Value::null || !doors.isArray()) {
    REPORT_ERROR("Config '%s' of type '%s' not found in '%s'.",
        "doors", "array", file.c_str());
    return false;
  }

  for (int i = 0; i < doors.size(); i++) {
    std::string name;
    if (!GetString(doors[i]["name"], &name)) {
      REPORT_ERROR("Config 'doors[%d].%s' of type '%s' not found in '%s'.",
          i, "name", "string", file.c_str());
      return false;
    }
    if (doors_.count(name) != 0) {
      REPORT_ERROR("Door '%s' defined twice in '%s'.",
          name.c_str(), file.c_str());
      return false;
    }
    doors_[name].name = name;

    if (!GetString(doors[i]["body"], &doors_[name].body_name)) {
      REPORT_ERROR("Config 'doors[%d].%s' of type '%s' not found in '%s'.",
          i, "body", "string", file.c_str());
      return false;
    }
    if (!GetString(doors[i]["sprite"], &doors_[name].sprite_name)) {
      REPORT_ERROR("Config 'doors[%d].%s' of type '%s' not found in '%s'.",
          i, "sprite", "string", file.c_str());
      return false;
    }
    if (!GetFloat32(doors[i]["activation_distance"],
                    &doors_[name].activation_distance)) {
      REPORT_ERROR("Config 'doors[%d].%s' of type '%s' not found in '%s'.",
          i, "activation_distance", "float", file.c_str());
      return false;
    }
  }

  return true;
}

bool Config::LoadKitsConfig() {
  std::string file = "data/entities.json";
  Json::Reader reader;
  Json::Value root;

  if (!ParseFile(file, &reader, &root)) {
      REPORT_ERROR("Can't parse file '%s'.", file.c_str());
      return false;
  }

  Json::Value kits = root["kits"];
  if (kits == Json::Value::null || !kits.isArray()) {
    REPORT_ERROR("Config '%s' of type '%s' not found in '%s'.",
        "kits", "array", file.c_str());
    return false;
  }

  for (int i = 0; i < kits.size(); i++) {
    std::string name;
    if (!GetString(kits[i]["name"], &name)) {
      REPORT_ERROR("Config 'kits[%d].%s' of type '%s' not found in '%s'.",
          i, "name", "string", file.c_str());
      return false;
    }
    if (kits_.count(name) != 0) {
      REPORT_ERROR("Critter '%s' defined twice in '%s'.",
          name.c_str(), file.c_str());
      return false;
    }
    kits_[name].name = name;

    if (!GetString(kits[i]["body"], &kits_[name].body_name)) {
      REPORT_ERROR("Config 'kits[%d].%s' of type '%s' not found in '%s'.",
          i, "body", "string", file.c_str());
      return false;
    }
    if (!GetString(kits[i]["sprite"], &kits_[name].sprite_name)) {
      REPORT_ERROR("Config 'kits[%d].%s' of type '%s' not found in '%s'.",
          i, "sprite", "string", file.c_str());
      return false;
    }
    if (!GetInt32(kits[i]["health_regen"], &kits_[name].health_regen)) {
      REPORT_ERROR("Config 'critters[%d].%s' of type '%s' not found in '%s'.",
          i, "health_regen", "int", file.c_str());
      return false;
    }
    if (!GetInt32(kits[i]["energy_regen"], &kits_[name].energy_regen)) {
      REPORT_ERROR("Config 'critters[%d].%s' of type '%s' not found in '%s'.",
          i, "energy_regen", "int", file.c_str());
      return false;
    }
  }

  return true;
}

bool Config::LoadPlayersConfig() {
  std::string file = "data/entities.json";
  Json::Reader reader;
  Json::Value root;

  if (!ParseFile(file, &reader, &root)) {
      REPORT_ERROR("Can't parse file '%s'.", file.c_str());
      return false;
  }

  Json::Value players = root["players"];
  if (players == Json::Value::null || !players.isArray()) {
    REPORT_ERROR("Config '%s' of type '%s' not found in '%s'.",
        "players", "array", file.c_str());
    return false;
  }

  for (int i = 0; i < players.size(); i++) {
    std::string name;
    if (!GetString(players[i]["name"], &name)) {
      REPORT_ERROR("Config 'players[%d].%s' of type '%s' not found in '%s'.",
          i, "name", "string", file.c_str());
      return false;
    }
    if (players_.count(name) != 0) {
      REPORT_ERROR("Player '%s' defined twice in '%s'.",
          name.c_str(), file.c_str());
      return false;
    }
    players_[name].name = name;

    if (!GetString(players[i]["body"], &players_[name].body_name)) {
      REPORT_ERROR("Config 'players[%d].%s' of type '%s' not found in '%s'.",
          i, "body", "string", file.c_str());
      return false;
    }
    if (!GetString(players[i]["sprite"], &players_[name].sprite_name)) {
      REPORT_ERROR("Config 'players[%d].%s' of type '%s' not found in '%s'.",
          i, "sprite", "string", file.c_str());
      return false;
    }
    if (!GetFloat32(players[i]["speed"], &players_[name].speed)) {
      REPORT_ERROR("Config 'players[%d].%s' of type '%s' not found in '%s'.",
          i, "speed", "int", file.c_str());
      return false;
    }
    if (!GetInt32(players[i]["health_max"], &players_[name].health_max)) {
      REPORT_ERROR("Config 'players[%d].%s' of type '%s' not found in '%s'.",
          i, "health_max", "int", file.c_str());
      return false;
    }
    if (!GetInt32(players[i]["health_regen"], &players_[name].health_regen)) {
      REPORT_ERROR("Config 'players[%d].%s' of type '%s' not found in '%s'.",
          i, "health_regen", "int", file.c_str());
      return false;
    }
    if (!GetInt32(players[i]["energy_max"], &players_[name].energy_max)) {
      REPORT_ERROR("Config 'players[%d].%s' of type '%s' not found in '%s'.",
          i, "energy_max", "int", file.c_str());
      return false;
    }
    if (!GetInt32(players[i]["energy_regen"], &players_[name].energy_regen)) {
      REPORT_ERROR("Config 'players[%d].%s' of type '%s' not found in '%s'.",
          i, "energy_regen", "int", file.c_str());
      return false;
    }
  }

  return true;
}

bool Config::LoadProjectilesConfig() {
  std::string file = "data/entities.json";
  Json::Reader reader;
  Json::Value root;

  if (!ParseFile(file, &reader, &root)) {
      REPORT_ERROR("Can't parse file '%s'.", file.c_str());
      return false;
  }

  Json::Value projectiles = root["projectiles"];
  if (projectiles == Json::Value::null || !projectiles.isArray()) {
    REPORT_ERROR("Config '%s' of type '%s' not found in '%s'.",
        "projectiles", "array", file.c_str());
    return false;
  }

  for (int i = 0; i < projectiles.size(); i++) {
    std::string name;
    if (!GetString(projectiles[i]["name"], &name)) {
      REPORT_ERROR("Config '%s[%d].%s' of type '%s' not found in '%s'.",
          "projectiles", i, "name", "string", file.c_str());
      return false;
    }
    if (projectiles_.count(name) != 0) {
      REPORT_ERROR("Projectile '%s' defined twice in '%s'.",
          name.c_str(), file.c_str());
      return false;
    }
    projectiles_[name].name = name;

    if (!GetString(projectiles[i]["body"], &projectiles_[name].body_name)) {
      REPORT_ERROR("Config '%s[%d].%s' of type '%s' not found in '%s'.",
          "projectiles", i, "body", "string", file.c_str());
      return false;
    }
    if (!GetString(projectiles[i]["sprite"], &projectiles_[name].sprite_name)) {
      REPORT_ERROR("Config '%s[%d].%s' of type '%s' not found in '%s'.",
          "projectiles", i, "sprite", "string", file.c_str());
      return false;
    }
    if (!GetFloat32(projectiles[i]["speed"], &projectiles_[name].speed)) {
      REPORT_ERROR("Config '%s[%d].%s' of type '%s' not found in '%s'.",
          "projectiles", i, "speed", "float", file.c_str());
      return false;
    }

    std::string type;
    if (!GetString(projectiles[i]["type"], &type)) {
      REPORT_ERROR("Config '%s[%d].%s' of type '%s' not found in '%s'.",
          "projectiles", i, "type", "string", file.c_str());
      return false;
    }
    if (type == "rocket") {
      projectiles_[name].type = ProjectileConfig::TYPE_ROCKET;
      if (!GetFloat32(projectiles[i]["explosion_radius"],
                      &projectiles_[name].rocket_config.explosion_radius)) {
        REPORT_ERROR("Config '%s[%d].%s' of type '%s' not found in '%s'.",
            "projectiles", i, "explosion_radius", "float", file.c_str());
        return false;
      }
      if (!GetInt32(projectiles[i]["explosion_damage"],
                      &projectiles_[name].rocket_config.explosion_damage)) {
        REPORT_ERROR("Config '%s[%d].%s' of type '%s' not found in '%s'.",
            "projectiles", i, "explosion_damage", "int", file.c_str());
        return false;
      }
    } else if (type == "slime") {
      projectiles_[name].type = ProjectileConfig::TYPE_SLIME;
      if (!GetInt32(projectiles[i]["explosion_radius"],
                      &projectiles_[name].slime_config.explosion_radius)) {
        REPORT_ERROR("Config '%s[%d].%s' of type '%s' not found in '%s'.",
            "projectiles", i, "explosion_radius", "int", file.c_str());
      }
    } else {
      REPORT_ERROR("Config %s[%d].%s must be 'rocket' or 'slime' in %s.",
          "projectiles", i, "type", file.c_str());
      return false;
    }
  }

  return true;
}

bool Config::LoadWallsConfig() {
  std::string file = "data/entities.json";
  Json::Reader reader;
  Json::Value root;

  if (!ParseFile(file, &reader, &root)) {
      REPORT_ERROR("Can't parse file '%s'.", file.c_str());
      return false;
  }

  Json::Value walls = root["walls"];
  if (walls == Json::Value::null || !walls.isArray()) {
    REPORT_ERROR("Config '%s' of type '%s' not found in '%s'.",
        "walls", "array", file.c_str());
    return false;
  }

  for (int i = 0; i < walls.size(); i++) {
    std::string name;
    if (!GetString(walls[i]["name"], &name)) {
      REPORT_ERROR("Config 'walls[%d].%s' of type '%s' not found in '%s'.",
          i, "name", "string", file.c_str());
      return false;
    }
    if (walls_.count(name) != 0) {
      REPORT_ERROR("Wall '%s' defined twice in '%s'.",
          name.c_str(), file.c_str());
      return false;
    }
    walls_[name].name = name;

    if (!GetString(walls[i]["body"], &walls_[name].body_name)) {
      REPORT_ERROR("Config '%s[%d].%s' of type '%s' not found in '%s'.",
          "walls", i, "body", "string", file.c_str());
      return false;
    }
    if (!GetString(walls[i]["sprite"], &walls_[name].sprite_name)) {
      REPORT_ERROR("Config '%s[%d].%s' of type '%s' not found in '%s'.",
          "walls", i, "sprite", "string", file.c_str());
      return false;
    }

    std::string type;
    if (!GetString(walls[i]["type"], &type)) {
      REPORT_ERROR("Config '%s[%d].%s' of type '%s' not found in '%s'.",
          "walls", i, "type", "string", file.c_str());
      return false;
    }
    if (type == "ordinary") {
      walls_[name].type = WallConfig::TYPE_ORDINARY;
    } else if (type == "unbreakable") {
      walls_[name].type = WallConfig::TYPE_UNBREAKABLE;
    } else {
      REPORT_ERROR("Config %s[%d].%s must be '%s' or '%s' in %s.",
          "walls", i, "type", "ordinary", "unbreakable", file.c_str());
      return false;
    }
  }

  return true;
}

bool Config::LoadGunsConfig() {
  std::string file = "data/guns.json";
  Json::Reader reader;
  Json::Value root;

  if (!ParseFile(file, &reader, &root)) {
      REPORT_ERROR("Can't parse file '%s'.", file.c_str());
      return false;
  }

  Json::Value guns = root["guns"];
  if (guns == Json::Value::null) {
    REPORT_ERROR("Config '%s' of type '%s' not found in '%s'.",
        "guns", "array", file.c_str());
    return false;
  }
  if (guns.size() == 0) {
    REPORT_ERROR("Array '%s' is empty in '%s'.", "guns", file.c_str());
    return false;
  }

  for (int i = 0; i < guns.size(); i++) {
    std::string name;
    if (!GetString(guns[i]["name"], &name)) {
      REPORT_ERROR("Config 'guns[%d].%s' of type '%s' not found in '%s'.",
          i, "name", "string", file.c_str());
      return false;
    }
    if (guns_.count(name) != 0) {
      REPORT_ERROR("Gun '%s' defined twice in '%s'.",
          name.c_str(), file.c_str());
      return false;
    }
    guns_[name].name = name;

    if (!GetString(guns[i]["projectile"], &guns_[name].projectile_name)) {
      REPORT_ERROR("Config 'guns[%d].%s' of type '%s' not found in '%s'.",
          i, "projectile", "string", file.c_str());
      return false;
    }
    if (!GetInt32(guns[i]["energy_consumption"],
                  &guns_[name].energy_consumption)) {
      REPORT_ERROR("Config 'guns[%d].%s' of type '%s' not found in '%s'.",
          i, "energy_consumption", "int", file.c_str());
      return false;
    }
  }

  return true;
}

}  // namespace bm
