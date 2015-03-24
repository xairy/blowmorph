// Copyright (c) 2015 Blowmorph Team

#ifndef ENGINE_MAP_H_
#define ENGINE_MAP_H_

#include <string>
#include <vector>

#include "base/pstdint.h"

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
    std::string entity_name;
  };

  struct Kit {
    int32_t x, y;
    std::string entity_name;
  };

  struct Wall {
    int32_t x, y;
    std::string entity_name;
  };

  Map();
  ~Map();

  bool Load(const std::string& file);

  float32_t GetBlockSize() const;
  int32_t GetWidth() const;
  int32_t GetHeight() const;

  const Terrain& GetTerrain() const;
  const std::vector<Spawn>& GetSpawns() const;

  const std::vector<Kit>& GetKits() const;
  const std::vector<Door>& GetDoors() const;
  const std::vector<Wall>& GetWalls() const;

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
