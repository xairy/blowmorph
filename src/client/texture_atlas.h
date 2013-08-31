#ifndef BLOWMORPH_CLIENT_TEXTURE_MANAGER_H_
#define BLOWMORPH_CLIENT_TEXTURE_MANAGER_H_

#include <map>
#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <SFML/Graphics.hpp>

#include <base/pstdint.h>

namespace bm {

typedef sf::Rect<int64_t> TileRect;
typedef std::vector<TileRect> TileSet;

class TextureAtlas {
public:
  ~TextureAtlas();

  sf::Texture* GetTexture() const;

  glm::vec2 GetSize() const;
  size_t GetTileCount() const;
  glm::vec2 GetTilePosition(size_t i) const;
  glm::vec2 GetTileSize(size_t i) const;

private:
  TextureAtlas();

  sf::Texture* texture;
  TileSet tileset;

  friend TextureAtlas* LoadTexture(
    const std::string& path,
    uint32_t transparentColor
  );
  friend TextureAtlas* LoadTileset(
    const std::string& path,
    uint32_t transparent_color,
    int64_t start_x, int64_t start_y,
    int64_t horizontal_step, int64_t vertical_step,
    int64_t tile_width, int64_t tile_height
  );
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
  int64_t start_x, int64_t start_y,
  int64_t horizontal_step, int64_t vertical_step,
  int64_t tile_width, int64_t tile_height
);

TileSet MakeSimpleTileset(
  int64_t start_x = 0, int64_t start_y = 0,
  int64_t horizontal_step = 0, int64_t vertical_step = 0,
  int64_t tile_width = 0, int64_t tile_height = 0,
  int64_t image_width = 0, int64_t image_height = 0
);

} // namespace bm

#endif // BLOWMORPH_CLIENT_TEXTURE_MANAGER_H_
