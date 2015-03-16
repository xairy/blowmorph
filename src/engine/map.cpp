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

  if (!GetInt32(root["map_size"], &size_)) {
    REPORT_ERROR("Config '%s' of type '%s' not found in '%s'.",
        "map_size", "int", file.c_str());
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

  Json::Value chunks = root["chunks"];
  if (chunks == Json::Value::null) {
    REPORT_ERROR("Config '%s' of type '%s' not found in '%s'.",
        "chunks", "array", file.c_str());
    return false;
  }
  if (chunks.size() == 0) {
    REPORT_ERROR("Array '%s' is empty in '%s'.", "chunks", file.c_str());
    return false;
  }

  for (int i = 0; i < chunks.size(); i++) {
    int32_t x, y;
    int32_t width, height;
    std::string entity_name;

    if (!GetInt32(chunks[i]["x"], &x)) {
      REPORT_ERROR("Config 'chunks[%d].x' of type '%s' not found in '%s'.",
          i, "int", file.c_str());
      return false;
    }
    if (!GetInt32(chunks[i]["y"], &y)) {
      REPORT_ERROR("Config 'chunks[%d].y' of type '%s' not found in '%s'.",
          i, "int", file.c_str());
      return false;
    }
    if (!GetInt32(chunks[i]["width"], &width)) {
      REPORT_ERROR("Config 'chunks[%d].width' of type '%s' not found in '%s'.",
          i, "int", file.c_str());
      return false;
    }
    if (!GetInt32(chunks[i]["height"], &height)) {
      REPORT_ERROR("Config 'chunks[%d].height' of type '%s' not found in '%s'.",
          i, "int", file.c_str());
      return false;
    }
    if (!GetString(chunks[i]["entity"], &entity_name)) {
      REPORT_ERROR("Config 'chunks[%d].entity' of type '%s' not found in '%s'.",
          i, "string", file.c_str());
      return false;
    }

    chunks_.push_back(Chunk{x, y, width, height, entity_name});
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

  return true;
}

int32_t Map::GetSize() const {
  return size_;
}

float32_t Map::GetBlockSize() const {
  return block_size_;
}

const std::vector<Map::Spawn>& Map::GetSpawns() const {
  return spawns_;
}

const std::vector<Map::Chunk>& Map::GetChunks() const {
  return chunks_;
}

const std::vector<Map::Kit>& Map::GetKits() const {
  return kits_;
}

}  // namespace bm
