// Copyright (c) 2013 Blowmorph Team

#ifndef CLIENT_TEXTURE_ATLAS_H_
#define CLIENT_TEXTURE_ATLAS_H_

#include <map>
#include <string>
#include <vector>

#include <SFML/Graphics.hpp>

#include "base/pstdint.h"

namespace bm {

typedef sf::Rect<int32_t> TileRect;
typedef std::vector<TileRect> TileSet;

class TextureAtlas {
 public:
  ~TextureAtlas();

  sf::Texture* GetTexture() const;

  sf::Vector2i GetSize() const;
  size_t GetTileCount() const;
  sf::Vector2i GetTilePosition(size_t i) const;
  sf::Vector2i GetTileSize(size_t i) const;

 private:
  TextureAtlas();

  sf::Texture* texture;
  TileSet tileset;

  friend TextureAtlas* LoadTexture(
    const std::string& path,
    uint32_t transparentColor);

  friend TextureAtlas* LoadTileset(
    const std::string& path,
    uint32_t transparent_color,
    int32_t start_x, int32_t start_y,
    int32_t horizontal_step, int32_t vertical_step,
    int32_t tile_width, int32_t tile_height);
};

// Loads an image or a tileset from a given path.
// Returns NULL on failure.
TextureAtlas* LoadTexture(
  const std::string& path,
  uint32_t transparentColor = 0xFFFFFFFF
);

// Loads a tileset from a given path.
// Returns NULL on failure.
TextureAtlas* LoadTileset(
  const std::string& path,
  uint32_t transparent_color,
  int32_t start_x, int32_t start_y,
  int32_t horizontal_step, int32_t vertical_step,
  int32_t tile_width, int32_t tile_height
);

TileSet MakeSimpleTileset(
  int32_t start_x = 0, int32_t start_y = 0,
  int32_t horizontal_step = 0, int32_t vertical_step = 0,
  int32_t tile_width = 0, int32_t tile_height = 0,
  int32_t image_width = 0, int32_t image_height = 0
);

}  // namespace bm

#endif  // CLIENT_TEXTURE_ATLAS_H_
