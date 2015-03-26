// Copyright (c) 2015 Blowmorph Team

#ifndef ENGINE_MAP_H_
#define ENGINE_MAP_H_

#include <string>
#include <vector>

#include "base/pstdint.h"

#include "engine/dll.h"

namespace bm {

class Map {
 public:
  struct Terrain {
    std::vector<std::string> sprite_names;
  };

  struct Spawn {
    int32_t x, y;
  };

  struct Door {
    int32_t x, y;
    int32_t rotation;
    std::string entity_name;
  };

  struct Kit {
    int32_t x, y;
    int32_t rotation;
    std::string entity_name;
  };

  struct Wall {
    int32_t x, y;
    int32_t rotation;
    std::string entity_name;
  };

  BM_ENGINE_DECL Map();
  BM_ENGINE_DECL ~Map();

  BM_ENGINE_DECL bool Load(const std::string& file);

  BM_ENGINE_DECL float32_t GetBlockSize() const;
  BM_ENGINE_DECL int32_t GetWidth() const;
  BM_ENGINE_DECL int32_t GetHeight() const;

  BM_ENGINE_DECL const Terrain& GetTerrain() const;
  BM_ENGINE_DECL const std::vector<Spawn>& GetSpawns() const;

  BM_ENGINE_DECL const std::vector<Kit>& GetKits() const;
  BM_ENGINE_DECL const std::vector<Door>& GetDoors() const;
  BM_ENGINE_DECL const std::vector<Wall>& GetWalls() const;

 private:
  float32_t block_size_;
  int32_t width_;
  int32_t height_;

  Terrain terrain_;
  std::vector<Spawn> spawns_;
  std::vector<Door> doors_;
  std::vector<Kit> kits_;
  std::vector<Wall> walls_;
};

}  // namespace bm

#endif  // ENGINE_MAP_H_
