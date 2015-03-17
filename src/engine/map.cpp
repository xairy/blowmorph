// Copyright (c) 2015 Blowmorph Team

#include "engine/map.h"

#include <fstream>  // NOLINT
#include <string>
#include <vector>

#include "base/error.h"
#include "base/json.h"
#include "base/pstdint.h"

namespace bm {

Map::Map() { }
Map::~Map() { }

bool Map::Load(const std::string& file) {
  std::fstream stream(file);
  if (!stream.is_open()) {
    REPORT_ERROR("Can't open file '%s'.", file.c_str());
    return false;
  }

  Json::Value root;
  Json::Reader reader;

  bool success = reader.parse(stream, root, false);
  if (!success) {
      std::string error = reader.getFormatedErrorMessages();
      REPORT_ERROR("Can't parse '%s':\n%s", file.c_str(), error.c_str());
      return false;
  }

  // Load globals.

  if (!GetFloat32(root["block_size"], &block_size_)) {
    REPORT_ERROR("Config '%s' of type '%s' not found in '%s'.",
        "block_size", "float", file.c_str());
    return false;
  }

  if (!GetInt32(root["width"], &width_)) {
    REPORT_ERROR("Config '%s' of type '%s' not found in '%s'.",
        "width", "int", file.c_str());
    return false;
  }

  if (!GetInt32(root["height"], &height_)) {
    REPORT_ERROR("Config '%s' of type '%s' not found in '%s'.",
        "height", "int", file.c_str());
    return false;
  }

  // Load spawns.

  Json::Value spawns = root["spawns"];
  if (spawns == Json::Value::null) {
    REPORT_ERROR("Config '%s' of type '%s' not found in '%s'.",
        "spawns", "array", file.c_str());
    return false;
  }
  if (spawns.size() == 0) {
    REPORT_ERROR("Array '%s' is empty in '%s'.", "spawns", file.c_str());
    return false;
  }

  for (int i = 0; i < spawns.size(); i++) {
    int32_t x, y;
    if (!GetInt32(spawns[i]["x"], &x)) {
      REPORT_ERROR("Config 'spawns[%d].x' of type '%s' not found in '%s'.",
          i, "int", file.c_str());
      return false;
    }
    if (!GetInt32(spawns[i]["y"], &y)) {
      REPORT_ERROR("Config 'spawns[%d].y' of type '%s' not found in '%s'.",
          i, "int", file.c_str());
      return false;
    }
    spawns_.push_back(Spawn{x, y});
  }

  // Load walls.

  Json::Value walls = root["walls"];
  if (walls == Json::Value::null) {
    REPORT_ERROR("Config '%s' of type '%s' not found in '%s'.",
        "walls", "array", file.c_str());
    return false;
  }
  if (walls.size() == 0) {
    REPORT_ERROR("Array '%s' is empty in '%s'.", "walls", file.c_str());
    return false;
  }

  for (int i = 0; i < walls.size(); i++) {
    int32_t x, y;
    std::string entity_name;

    if (!GetInt32(walls[i]["x"], &x)) {
      REPORT_ERROR("Config 'walls[%d].x' of type '%s' not found in '%s'.",
          i, "int", file.c_str());
      return false;
    }
    if (!GetInt32(walls[i]["y"], &y)) {
      REPORT_ERROR("Config 'walls[%d].y' of type '%s' not found in '%s'.",
          i, "int", file.c_str());
      return false;
    }
    if (!GetString(walls[i]["entity"], &entity_name)) {
      REPORT_ERROR("Config 'walls[%d].entity' of type '%s' not found in '%s'.",
          i, "string", file.c_str());
      return false;
    }

    walls_.push_back(Wall{x, y, entity_name});
  }

  // Load kits.

  Json::Value kits = root["kits"];
  if (kits == Json::Value::null) {
    REPORT_ERROR("Config '%s' of type '%s' not found in '%s'.",
        "kits", "array", file.c_str());
    return false;
  }
  if (kits.size() == 0) {
    REPORT_ERROR("Array '%s' is empty in '%s'.", "kits", file.c_str());
    return false;
  }

  for (int i = 0; i < kits.size(); i++) {
    int32_t x, y;
    std::string entity_name;

    if (!GetInt32(kits[i]["x"], &x)) {
      REPORT_ERROR("Config 'kits[%d].x' of type '%s' not found in '%s'.",
          i, "int", file.c_str());
      return false;
    }
    if (!GetInt32(kits[i]["y"], &y)) {
      REPORT_ERROR("Config 'kits[%d].y' of type '%s' not found in '%s'.",
          i, "int", file.c_str());
      return false;
    }
    if (!GetString(kits[i]["entity"], &entity_name)) {
      REPORT_ERROR("Config 'kits[%d].entity' of type '%s' not found in '%s'.",
          i, "string", file.c_str());
      return false;
    }

    kits_.push_back(Kit{x, y, entity_name});
  }

  // Load terrain.

  Json::Value terrain = root["terrain"];
  if (terrain == Json::Value::null) {
    REPORT_ERROR("Config '%s' of type '%s' not found in '%s'.",
        "terrain", "array", file.c_str());
    return false;
  }
  if (terrain.size() != width_ * height_) {
    REPORT_ERROR("Size of array '%s' must be '%d' in %s'.",
      "terrain", width_ * height_, file.c_str());
    return false;
  }

  for (int i = 0; i < terrain.size(); i++) {
    std::string sprite_name;

    if (!GetString(terrain[i], &sprite_name)) {
      REPORT_ERROR("Config 'terrain[%d]' of type '%s' not found in '%s'.",
          i, "string", file.c_str());
      return false;
    }

    terrain_.sprite_names.push_back(sprite_name);
  }

  return true;
}

float32_t Map::GetBlockSize() const {
  return block_size_;
}

int32_t Map::GetWidth() const {
  return width_;
}

int32_t Map::GetHeight() const {
  return height_;
}

const std::vector<Map::Spawn>& Map::GetSpawns() const {
  return spawns_;
}

const std::vector<Map::Wall>& Map::GetWalls() const {
  return walls_;
}

const std::vector<Map::Kit>& Map::GetKits() const {
  return kits_;
}

const Map::Terrain& Map::GetTerrain() const {
  return terrain_;
}

}  // namespace bm
