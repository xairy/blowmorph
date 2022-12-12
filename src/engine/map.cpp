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
  Json::Reader reader;
  Json::Value root;

  if (!ParseFile(file, &reader, &root)) {
      REPORT_ERROR("Can't parse file '%s'.", file.c_str());
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

  // Load terrain.

  Json::Value terrain = root["terrain"];
  if (terrain.isNull()) {
    REPORT_ERROR("Config '%s' of type '%s' not found in '%s'.",
        "terrain", "array", file.c_str());
    return false;
  }
  if (terrain.size() != width_ * height_) {
    REPORT_ERROR("Size of array '%s' must be '%d' in %s'.",
      "terrain", width_ * height_, file.c_str());
    return false;
  }

  for (int i = 0; i < static_cast<int>(terrain.size()); i++) {
    std::string sprite_name;

    if (!GetString(terrain[i], &sprite_name)) {
      REPORT_ERROR("Config 'terrain[%d]' of type '%s' not found in '%s'.",
          i, "string", file.c_str());
      return false;
    }

    terrain_.sprite_names.push_back(sprite_name);
  }

  // Load spawns.

  Json::Value spawns = root["spawns"];
  if (spawns.isNull()) {
    REPORT_ERROR("Config '%s' of type '%s' not found in '%s'.",
        "spawns", "array", file.c_str());
    return false;
  }
  if (spawns.size() == 0) {
    REPORT_ERROR("Array '%s' is empty in '%s'.", "spawns", file.c_str());
    return false;
  }

  for (int i = 0; i < static_cast<int>(spawns.size()); i++) {
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
    spawns_.push_back(Spawn {x, y});
  }

  // Load zombie spawns.

  Json::Value zombie_spawns = root["zombie_spawns"];
  if (zombie_spawns.isNull()) {
    REPORT_ERROR("Config '%s' of type '%s' not found in '%s'.",
        "zombie_spawns", "array", file.c_str());
    return false;
  }

  for (int i = 0; i < static_cast<int>(zombie_spawns.size()); i++) {
    int32_t x, y;
    if (!GetInt32(zombie_spawns[i]["x"], &x)) {
      REPORT_ERROR("Config 'zombie_spawns[%d].x' of type '%s' not found in '%s'.",
          i, "int", file.c_str());
      return false;
    }
    if (!GetInt32(zombie_spawns[i]["y"], &y)) {
      REPORT_ERROR("Config 'zombie_spawns[%d].y' of type '%s' not found in '%s'.",
          i, "int", file.c_str());
      return false;
    }
    zombie_spawns_.push_back(Spawn {x, y});
  }

  // Load doors.

  Json::Value doors = root["doors"];
  if (doors.isNull()) {
    REPORT_ERROR("Config '%s' of type '%s' not found in '%s'.",
        "doors", "array", file.c_str());
    return false;
  }

  for (int i = 0; i < static_cast<int>(doors.size()); i++) {
    int32_t x, y;
    int32_t rotation;
    std::string entity_name;

    if (!GetInt32(doors[i]["x"], &x)) {
      REPORT_ERROR("Config 'doors[%d].x' of type '%s' not found in '%s'.",
          i, "int", file.c_str());
      return false;
    }
    if (!GetInt32(doors[i]["y"], &y)) {
      REPORT_ERROR("Config 'doors[%d].y' of type '%s' not found in '%s'.",
          i, "int", file.c_str());
      return false;
    }
    if (!GetInt32(doors[i]["rotation"], &rotation)) {
      rotation = 0;
    }
    if (!GetString(doors[i]["entity"], &entity_name)) {
      REPORT_ERROR("Config 'doors[%d].entity' of type '%s' not found in '%s'.",
          i, "string", file.c_str());
      return false;
    }

    doors_.push_back(Door {x, y, rotation, entity_name});
  }

  // Load kits.

  Json::Value kits = root["kits"];
  if (kits.isNull()) {
    REPORT_ERROR("Config '%s' of type '%s' not found in '%s'.",
        "kits", "array", file.c_str());
    return false;
  }

  for (int i = 0; i < static_cast<int>(kits.size()); i++) {
    int32_t x, y;
    int32_t rotation;
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
    if (!GetInt32(kits[i]["rotation"], &rotation)) {
      rotation = 0;
    }
    if (!GetString(kits[i]["entity"], &entity_name)) {
      REPORT_ERROR("Config 'kits[%d].entity' of type '%s' not found in '%s'.",
          i, "string", file.c_str());
      return false;
    }

    kits_.push_back(Kit {x, y, rotation, entity_name});
  }

  // Load walls.

  Json::Value walls = root["walls"];
  if (walls.isNull()) {
    REPORT_ERROR("Config '%s' of type '%s' not found in '%s'.",
        "walls", "array", file.c_str());
    return false;
  }

  for (int i = 0; i < static_cast<int>(walls.size()); i++) {
    int32_t x, y;
    int32_t rotation;
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
    if (!GetInt32(walls[i]["rotation"], &rotation)) {
      rotation = 0;
    }
    if (!GetString(walls[i]["entity"], &entity_name)) {
      REPORT_ERROR("Config 'walls[%d].entity' of type '%s' not found in '%s'.",
          i, "string", file.c_str());
      return false;
    }

    walls_.push_back(Wall {x, y, rotation, entity_name});
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

const Map::Terrain& Map::GetTerrain() const {
  return terrain_;
}

const std::vector<Map::Spawn>& Map::GetSpawns() const {
  return spawns_;
}

const std::vector<Map::Spawn>& Map::GetZombieSpawns() const {
  return zombie_spawns_;
}

const std::vector<Map::Door>& Map::GetDoors() const {
  return doors_;
}

const std::vector<Map::Kit>& Map::GetKits() const {
  return kits_;
}

const std::vector<Map::Wall>& Map::GetWalls() const {
  return walls_;
}

}  // namespace bm
