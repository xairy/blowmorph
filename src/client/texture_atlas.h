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
  TextureAtlas();
  ~TextureAtlas();

  bool LoadTexture(
    const std::string& path,
    uint32_t transparent_color);

  bool LoadTileset(
    const std::string& path,
    uint32_t transparent_color,
    int32_t start_x, int32_t start_y,
    int32_t horizontal_step, int32_t vertical_step,
    int32_t tile_width, int32_t tile_height);

  void Finalize();

  sf::Texture* GetTexture() const;
  sf::Vector2i GetSize() const;

  size_t GetTileCount() const;
  sf::Vector2i GetTilePosition(size_t i) const;
  sf::Vector2i GetTileSize(size_t i) const;

 private:
  sf::Texture* texture_;
  TileSet tileset_;

  enum {
    STATE_FINALIZED,
    STATE_INITIALIZED
  } state_;

  DISALLOW_COPY_AND_ASSIGN(TextureAtlas);
};

}  // namespace bm

#endif  // CLIENT_TEXTURE_ATLAS_H_
