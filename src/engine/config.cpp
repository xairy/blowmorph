// Copyright (c) 2015 Blowmorph Team

#include "engine/config.h"

#include <map>
#include <string>
#include <fstream>  // NOLINT
#include <vector>

#include "base/config_reader.h"
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
  if (!LoadActivatorsConfig() || !LoadCrittersConfig() || !LoadKitsConfig() ||
      !LoadPlayersConfig() || !LoadProjectilesConfig() || !LoadWallsConfig()) {
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
  ConfigReader reader;
  const char* file = "data/master-server.cfg";
  if (!reader.Open(file)) {
    return false;
  }
  if (!reader.LookupUInt16("master-server.port", &master_server_.port)) {
    REPORT_ERROR("Unable to load 'master-server.port' from '%s'.", file);
    return false;
  }
  return true;
}

bool Config::LoadServerConfig() {
  ConfigReader reader;
  const char* file = "data/server.cfg";
  if (!reader.Open(file)) {
    return false;
  }
  if (!reader.LookupUInt16("server.port", &server_.port)) {
    REPORT_ERROR("Unable to load 'server.port' from '%s'.", file);
    return false;
  }
  if (!reader.LookupInt32("server.broadcast_rate", &server_.broadcast_rate)) {
    REPORT_ERROR("Unable to load 'server.broadcast_rate' from '%s'.", file);
    return false;
  }
  if (!reader.LookupInt32("server.tick_rate", &server_.tick_rate)) {
    REPORT_ERROR("Unable to load 'server.tick_rate' from '%s'.", file);
    return false;
  }
  if (!reader.LookupString("server.map", &server_.map)) {
    REPORT_ERROR("Unable to load 'server.map' from '%s'.", file);
    return false;
  }
  if (!reader.LookupString("server.name", &server_.name)) {
    REPORT_ERROR("Unable to load 'server.name' from '%s'.", file);
    return false;
  }
  if (!reader.LookupString("master-server.host", &server_.master_server_host)) {
    REPORT_ERROR("Unable to load 'master-server.host' from '%s'.", file);
    return false;
  }
  if (!reader.LookupUInt16("master-server.port", &server_.master_server_port)) {
    REPORT_ERROR("Unable to load 'master-server.port' from '%s'.", file);
    return false;
  }
  return true;
}

bool Config::LoadClientConfig() {
  ConfigReader reader;
  const char* file = "data/client.cfg";
  if (!reader.Open(file)) {
    return false;
  }
  if (!reader.LookupString("server.host", &client_.server_host)) {
    REPORT_ERROR("Unable to load 'server.host' from '%s'.", file);
    return false;
  }
  if (!reader.LookupUInt16("server.port", &client_.server_port)) {
    REPORT_ERROR("Unable to load 'server.port' from '%s'.", file);
    return false;
  }
  if (!reader.LookupString("master-server.host", &client_.master_server_host)) {
    REPORT_ERROR("Unable to load 'master-server.host' from '%s'.", file);
    return false;
  }
  if (!reader.LookupUInt16("master-server.port", &client_.master_server_port)) {
    REPORT_ERROR("Unable to load 'master-server.port' from '%s'.", file);
    return false;
  }
  if (!reader.LookupString("player.login", &client_.player_name)) {
    REPORT_ERROR("Unable to load 'player.login' from '%s'.", file);
    return false;
  }
  if (!reader.LookupInt32("graphics.width", &client_.screen_width)) {
    REPORT_ERROR("Unable to load 'graphics.width' from '%s'.", file);
    return false;
  }
  if (!reader.LookupInt32("graphics.height", &client_.screen_height)) {
    REPORT_ERROR("Unable to load 'graphics.height' from '%s'.", file);
    return false;
  }
  if (!reader.LookupBool("graphics.fullscreen", &client_.fullscreen)) {
    REPORT_ERROR("Unable to load 'fullscreen' from '%s'.", file);
    return false;
  }
  if (!reader.LookupInt32("client.tick_rate", &client_.tick_rate)) {
    REPORT_ERROR("Unable to load 'client.tick_rate' from '%s'.", file);
    return false;
  }
  if (!reader.LookupInt32("client.connect_timeout", &client_.connect_timeout)) {
    REPORT_ERROR("Unable to load 'client.connect_timeout' from '%s'.", file);
    return false;
  }
  if (!reader.LookupInt32("client.sync_timeout", &client_.sync_timeout)) {
    REPORT_ERROR("Unable to load 'client.sync_timeout' from '%s'.", file);
    return false;
  }
  if (!reader.LookupFloat("client.max_player_misposition",
        &client_.max_player_misposition)) {
    REPORT_ERROR("Unable to load 'client.max_player_misposition' from '%s'.",
                file);
    return false;
  }
  if (!reader.LookupInt32("client.interpolation_offset",
        &client_.interpolation_offset)) {
    REPORT_ERROR("Unable to load 'client.interpolation_offset' from '%s'.",
                file);
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
  if (bodies == Json::Value::null) {
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
        REPORT_ERROR("Config '%s[%d][%s].%s' of type '%s' not found in '%s'.",
            "bodies", i, "shape", "width", "float", file.c_str());
        return false;
      }
      if (!GetFloat32(shape["height"], &bodies_[name].box_config.height)) {
        REPORT_ERROR("Config '%s[%d][%s].%s' of type '%s' not found in '%s'.",
            "bodies", i, "shape", "height", "float", file.c_str());
        return false;
      }
    } else if (type == "circle") {
      bodies_[name].shape_type = Config::BodyConfig::SHAPE_TYPE_CIRCLE;
      if (!GetFloat32(shape["radius"], &bodies_[name].circle_config.radius)) {
        REPORT_ERROR("Config '%s[%d][%s].%s' of type '%s' not found in '%s'.",
            "bodies", i, "shape", "radius", "float", file.c_str());
        return false;
      }
    } else {
      REPORT_ERROR("Config bodies[%d][%s].%s must be 'box' or 'circle' in %s.",
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
  if (textures == Json::Value::null) {
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
    textures_[name].tiled = true;

    if (!GetInt32(tile["start_x"], &textures_[name].tile_start_x)) {
      REPORT_ERROR("Config '%s[%d][%s].%s' of type '%s' not found in '%s'.",
          "textures", i, "tile", "start_x", "int", file.c_str());
      return false;
    }
    if (!GetInt32(tile["start_y"], &textures_[name].tile_start_y)) {
      REPORT_ERROR("Config '%s[%d][%s].%s' of type '%s' not found in '%s'.",
          "textures", i, "tile", "start_y", "int", file.c_str());
      return false;
    }
    if (!GetInt32(tile["step_x"], &textures_[name].tile_step_x)) {
      REPORT_ERROR("Config '%s[%d][%s].%s' of type '%s' not found in '%s'.",
          "textures", i, "tile", "step_x", "int", file.c_str());
      return false;
    }
    if (!GetInt32(tile["step_y"], &textures_[name].tile_step_y)) {
      REPORT_ERROR("Config '%s[%d][%s].%s' of type '%s' not found in '%s'.",
          "textures", i, "tile", "step_y", "int", file.c_str());
      return false;
    }
    if (!GetInt32(tile["width"], &textures_[name].tile_width)) {
      REPORT_ERROR("Config '%s[%d][%s].%s' of type '%s' not found in '%s'.",
          "textures", i, "tile", "width", "int", file.c_str());
      return false;
    }
    if (!GetInt32(tile["height"], &textures_[name].tile_height)) {
      REPORT_ERROR("Config '%s[%d][%s].%s' of type '%s' not found in '%s'.",
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
  if (sprites == Json::Value::null) {
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
      REPORT_ERROR("Config '%s[%d][%s].%s' of type '%s' not found in '%s'.",
          "sprites", i, "mode", "tiles", "array", file.c_str());
      return false;
    }
    for (int j = 0; j < tiles.size(); j++) {
      int tile;
      if (!GetInt32(tiles[j], &tile)) {
        REPORT_ERROR(
            "Config '%s[%d][%s][%s][%d]' of type '%s' not found in '%s'.",
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
  std::string file = "data/activators.json";
  Json::Reader reader;
  Json::Value root;

  if (!ParseFile(file, &reader, &root)) {
      REPORT_ERROR("Can't parse file '%s'.", file.c_str());
      return false;
  }

  Json::Value activators = root["activators"];
  if (activators == Json::Value::null) {
    REPORT_ERROR("Config '%s' of type '%s' not found in '%s'.",
        "activators", "array", file.c_str());
    return false;
  }
  if (activators.size() == 0) {
    REPORT_ERROR("Array '%s' is empty in '%s'.", "activators", file.c_str());
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

    std::string type;
    if (!GetString(activators[i]["type"], &type)) {
      REPORT_ERROR("Config 'activators[%d].%s' of type '%s' not found in '%s'.",
          i, "type", "string", file.c_str());
      return false;
    }
    if (type == "door") {
      activators_[name].type = ActivatorConfig::TYPE_DOOR;
    } else {
      REPORT_ERROR("Config activators[%d].%s must be 'door' in %s.",
          i, "type", file.c_str());
      return false;
    }
  }

  return true;
}

bool Config::LoadCrittersConfig() {
  ConfigReader reader;
  const char* file = "data/critters.cfg";
  if (!reader.Open(file)) {
    return false;
  }
  std::vector<std::string> names;
  reader.GetRootConfigs(&names);
  for (auto name : names) {
    critters_[name].name = name;

    std::string config = name + ".body";
    if (!reader.LookupString(config, &critters_[name].body_name)) {
      REPORT_ERROR("Unable to load '%s' from '%s'.", config.c_str(), file);
      return false;
    }
    if (bodies_.count(critters_[name].body_name) == 0) {
      REPORT_ERROR("Body '%s' used by critter '%s' not defined.",
        critters_[name].body_name.c_str(), name.c_str());
      return false;
    }

    config = name + ".sprite";
    if (!reader.LookupString(config, &critters_[name].sprite_name)) {
      REPORT_ERROR("Unable to load '%s' from '%s'.", config.c_str(), file);
      return false;
    }
    if (sprites_.count(critters_[name].sprite_name) == 0) {
      REPORT_ERROR("Sprite '%s' used by critter '%s' not defined.",
        critters_[name].sprite_name.c_str(), name.c_str());
      return false;
    }

    config = name + ".speed";
    if (!reader.LookupFloat(config, &critters_[name].speed)) {
      REPORT_ERROR("Unable to load '%s' from '%s'.", config.c_str(), file);
      return false;
    }
    config = name + ".damage";
    if (!reader.LookupInt32(config, &critters_[name].damage)) {
      REPORT_ERROR("Unable to load '%s' from '%s'.", config.c_str(), file);
      return false;
    }

    std::string type;
    config = name + ".type";
    if (!reader.LookupString(config, &type)) {
      REPORT_ERROR("Unable to load '%s' from '%s'.", config.c_str(), file);
      return false;
    }
    if (type == "zombie") {
      critters_[name].type = CritterConfig::TYPE_ZOMBIE;
    } else {
      REPORT_ERROR("Critter type must be 'zombie'.");
      return false;
    }
  }
  return true;
}

bool Config::LoadKitsConfig() {
  ConfigReader reader;
  const char* file = "data/kits.cfg";
  if (!reader.Open(file)) {
    return false;
  }
  std::vector<std::string> names;
  reader.GetRootConfigs(&names);
  for (auto name : names) {
    kits_[name].name = name;

    std::string config = name + ".body";
    if (!reader.LookupString(config, &kits_[name].body_name)) {
      REPORT_ERROR("Unable to load '%s' from '%s'.", config.c_str(), file);
      return false;
    }
    if (bodies_.count(kits_[name].body_name) == 0) {
      REPORT_ERROR("Body '%s' used by kit '%s' not defined.",
        kits_[name].body_name.c_str(), name.c_str());
      return false;
    }

    config = name + ".sprite";
    if (!reader.LookupString(config, &kits_[name].sprite_name)) {
      REPORT_ERROR("Unable to load '%s' from '%s'.", config.c_str(), file);
      return false;
    }
    if (sprites_.count(kits_[name].sprite_name) == 0) {
      REPORT_ERROR("Sprite '%s' used by kit '%s' not defined.",
        kits_[name].sprite_name.c_str(), name.c_str());
      return false;
    }

    std::string type;
    config = name + ".type";
    if (!reader.LookupString(config, &type)) {
      REPORT_ERROR("Unable to load '%s' from '%s'.", config.c_str(), file);
      return false;
    }
    if (type == "health") {
      kits_[name].type = KitConfig::TYPE_HEALTH;
    } else if (type == "energy") {
      kits_[name].type = KitConfig::TYPE_ENERGY;
    } else if (type == "composite") {
      kits_[name].type = KitConfig::TYPE_COMPOSITE;
    } else {
      REPORT_ERROR("Kit type must be 'health', 'energy' or 'composite'.");
      return false;
    }

    config = name + ".health_regeneration";
    if (!reader.LookupInt32(config, &kits_[name].health_regen)) {
      REPORT_ERROR("Unable to load '%s' from '%s'.", config.c_str(), file);
      return false;
    }
    config = name + ".energy_regeneration";
    if (!reader.LookupInt32(config, &kits_[name].energy_regen)) {
      REPORT_ERROR("Unable to load '%s' from '%s'.", config.c_str(), file);
      return false;
    }
  }
  return true;
}

bool Config::LoadPlayersConfig() {
  ConfigReader reader;
  const char* file = "data/players.cfg";
  if (!reader.Open(file)) {
    return false;
  }
  std::vector<std::string> names;
  reader.GetRootConfigs(&names);
  for (auto name : names) {
    players_[name].name = name;

    std::string config = name + ".body";
    if (!reader.LookupString(config, &players_[name].body_name)) {
      REPORT_ERROR("Unable to load '%s' from '%s'.", config.c_str(), file);
      return false;
    }
    if (bodies_.count(players_[name].body_name) == 0) {
      REPORT_ERROR("Body '%s' used by player '%s' not defined.",
        players_[name].body_name.c_str(), name.c_str());
      return false;
    }

    config = name + ".sprite";
    if (!reader.LookupString(config, &players_[name].sprite_name)) {
      REPORT_ERROR("Unable to load '%s' from '%s'.", config.c_str(), file);
      return false;
    }
    if (sprites_.count(players_[name].sprite_name) == 0) {
      REPORT_ERROR("Sprite '%s' used by player '%s' not defined.",
        players_[name].sprite_name.c_str(), name.c_str());
      return false;
    }

    config = name + ".speed";
    if (!reader.LookupFloat(config, &players_[name].speed)) {
      REPORT_ERROR("Unable to load '%s' from '%s'.", config.c_str(), file);
      return false;
    }
    config = name + ".max_health";
    if (!reader.LookupInt32(config, &players_[name].health_max)) {
      REPORT_ERROR("Unable to load '%s' from '%s'.", config.c_str(), file);
      return false;
    }
    config = name + ".health_regeneration";
    if (!reader.LookupInt32(config, &players_[name].health_regen)) {
      REPORT_ERROR("Unable to load '%s' from '%s'.", config.c_str(), file);
      return false;
    }
    config = name + ".energy_capacity";
    if (!reader.LookupInt32(config, &players_[name].energy_max)) {
      REPORT_ERROR("Unable to load '%s' from '%s'.", config.c_str(), file);
      return false;
    }
    config = name + ".energy_regeneration";
    if (!reader.LookupInt32(config, &players_[name].energy_regen)) {
      REPORT_ERROR("Unable to load '%s' from '%s'.", config.c_str(), file);
      return false;
    }
  }
  return true;
}

bool Config::LoadProjectilesConfig() {
  ConfigReader reader;
  const char* file = "data/projectiles.cfg";
  if (!reader.Open(file)) {
    return false;
  }
  std::vector<std::string> names;
  reader.GetRootConfigs(&names);
  for (auto name : names) {
    projectiles_[name].name = name;

    std::string config = name + ".body";
    if (!reader.LookupString(config, &projectiles_[name].body_name)) {
      REPORT_ERROR("Unable to load '%s' from '%s'.", config.c_str(), file);
      return false;
    }
    if (bodies_.count(projectiles_[name].body_name) == 0) {
      REPORT_ERROR("Body '%s' used by projectile '%s' not defined.",
        projectiles_[name].body_name.c_str(), name.c_str());
      return false;
    }

    config = name + ".sprite";
    if (!reader.LookupString(config, &projectiles_[name].sprite_name)) {
      REPORT_ERROR("Unable to load '%s' from '%s'.", config.c_str(), file);
      return false;
    }
    if (sprites_.count(projectiles_[name].sprite_name) == 0) {
      REPORT_ERROR("Sprite '%s' used by projectile '%s' not defined.",
        projectiles_[name].sprite_name.c_str(), name.c_str());
      return false;
    }

    config = name + ".speed";
    if (!reader.LookupFloat(config, &projectiles_[name].speed)) {
      REPORT_ERROR("Unable to load '%s' from '%s'.", config.c_str(), file);
      return false;
    }

    std::string type;
    config = name + ".type";
    if (!reader.LookupString(config, &type)) {
      REPORT_ERROR("Unable to load '%s' from '%s'.", config.c_str(), file);
      return false;
    }
    if (type == "rocket") {
      projectiles_[name].type = ProjectileConfig::TYPE_ROCKET;
      config = name + ".explosion_radius";
      if (!reader.LookupFloat(config,
            &projectiles_[name].rocket_config.explosion_radius)) {
        REPORT_ERROR("Unable to load '%s' from '%s'.", config.c_str(), file);
        return false;
      }
      config = name + ".explosion_damage";
      if (!reader.LookupInt32(config,
            &projectiles_[name].rocket_config.explosion_damage)) {
        REPORT_ERROR("Unable to load '%s' from '%s'.", config.c_str(), file);
        return false;
      }
    } else if (type == "slime") {
      projectiles_[name].type = ProjectileConfig::TYPE_SLIME;
      config = name + ".explosion_radius";
      if (!reader.LookupInt32(config,
            &projectiles_[name].slime_config.explosion_radius)) {
        REPORT_ERROR("Unable to load '%s' from '%s'.", config.c_str(), file);
        return false;
      }
    } else {
      REPORT_ERROR("Projectile type must be 'rocket' or 'slime'.");
      return false;
    }
  }
  return true;
}

bool Config::LoadWallsConfig() {
  ConfigReader reader;
  const char* file = "data/walls.cfg";
  if (!reader.Open(file)) {
    return false;
  }
  std::vector<std::string> names;
  reader.GetRootConfigs(&names);
  for (auto name : names) {
    walls_[name].name = name;

    std::string config = name + ".body";
    if (!reader.LookupString(config, &walls_[name].body_name)) {
      REPORT_ERROR("Unable to load '%s' from '%s'.", config.c_str(), file);
      return false;
    }
    if (bodies_.count(walls_[name].body_name) == 0) {
      REPORT_ERROR("Body '%s' used by wall '%s' not defined.",
        walls_[name].body_name.c_str(), name.c_str());
      return false;
    }

    config = name + ".sprite";
    if (!reader.LookupString(config, &walls_[name].sprite_name)) {
      REPORT_ERROR("Unable to load '%s' from '%s'.", config.c_str(), file);
      return false;
    }
    if (sprites_.count(walls_[name].sprite_name) == 0) {
      REPORT_ERROR("Sprite '%s' used by wall '%s' not defined.",
        walls_[name].sprite_name.c_str(), name.c_str());
      return false;
    }

    std::string type;
    config = name + ".type";
    if (!reader.LookupString(config, &type)) {
      REPORT_ERROR("Unable to load '%s' from '%s'.", config.c_str(), file);
      return false;
    }
    if (type == "ordinary") {
      walls_[name].type = WallConfig::TYPE_ORDINARY;
    } else if (type == "unbreakable") {
      walls_[name].type = WallConfig::TYPE_UNBREAKABLE;
    } else if (type == "morphed") {
      walls_[name].type = WallConfig::TYPE_MORPHED;
    } else {
      REPORT_ERROR("Wall type must be 'ordinary', 'unbreakable' or 'morphed'.");
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
