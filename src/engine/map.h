// Copyright (c) 2015 Blowmorph Team

#ifndef ENGINE_MAP_H_
#define ENGINE_MAP_H_

#include <string>
#include <vector>

#include "base/pstdint.h"

namespace bm {

class Map {
 public:
  struct Spawn {
    int32_t x, y;
  };

  struct Wall {
    int32_t x, y;
    std::string entity_name;
  };

  struct Kit {
    int32_t x, y;
    std::string entity_name;
  };

  struct Terrain {
    std::vector<std::string> sprite_names;
  };

  Map();
  ~Map();

  bool Load(const std::string& file);

  int32_t GetSize() const;
  float32_t GetBlockSize() const;

  const std::vector<Spawn>& GetSpawns() const;

  const std::vector<Wall>& GetWalls() const;
  const std::vector<Kit>& GetKits() const;

  const Terrain& GetTerrain() const;

 private:
  int32_t size_;
  float32_t block_size_;

  std::vector<Spawn> spawns_;
  std::vector<Wall> walls_;
  std::vector<Kit> kits_;

  Terrain terrain_;
};

}  // namespace bm

#endif  // ENGINE_MAP_H_
