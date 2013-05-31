#include "texture_atlas.hpp"

#include <map>
#include <string>

#include <base/error.hpp>
#include <base/pstdint.hpp>

// TODO[24.7.2012 alex]: better think of something like "ResourceManager"

namespace bm {

sf::Texture* TextureAtlas::GetTexture() const {
  return texture;
}

size_t TextureAtlas::GetTileCount() const {
  CHECK(texture != 0);
  return tileset.size();
}

glm::vec2 TextureAtlas::GetTilePosition(size_t i) const {
  CHECK(texture != 0);
  CHECK(i < GetTileCount());

  return glm::vec2(tileset[i].x, tileset[i].y);
}
glm::vec2 TextureAtlas::GetTileSize(size_t i) const {
  CHECK(texture != NULL);
  CHECK(i < GetTileCount());

  return glm::vec2(tileset[i].width, tileset[i].height);
}

glm::vec2 TextureAtlas::GetSize() const {
  CHECK(texture != NULL);
  return glm::vec2(texture->getSize().x, texture->getSize().y);
}

TextureAtlas::TextureAtlas() { }
TextureAtlas::~TextureAtlas() {
  if (texture != NULL) {
    delete texture;
    texture = NULL;
  }
}

TextureAtlas* LoadOldTexture(const std::string& path,
                        uint32_t transparentColor) {
  sf::Image image;
  if (!image.loadFromFile(path)) {
    BM_ERROR("Unable to load texture.");
    return NULL;
  }

  if (transparentColor != 0xFFFFFFFF) {
    transparentColor &= 0x00FFFFFF;
    uint8_t r, g, b;
    r = (transparentColor >> 16) & 0xFF;
    g = (transparentColor >> 8) & 0xFF;
    b = (transparentColor >> 0) & 0xFF;
    image.createMaskFromColor(sf::Color(r, g, b));
  }

  TextureAtlas* result = new TextureAtlas();
  CHECK(result != NULL);

  result->texture = new sf::Texture();
  CHECK(result->texture != NULL);
  result->texture->update(image);

  result->tileset.push_back(TileRect(0, 0, image.getSize().x, image.getSize().y));

  return result;
}

TextureAtlas* LoadTileset(const std::string& path,
                     uint32_t transparentColor,
                     size_t startX, size_t startY,
                     size_t horizontalStep, size_t verticalStep,
                     size_t tileWidth, size_t tileHeight) {
  TextureAtlas* result = LoadOldTexture(path, transparentColor);
  if(result == NULL) {
    return NULL;
  }

  result->tileset.clear();
  result->tileset = MakeSimpleTileset(startX, startY,
    horizontalStep, verticalStep,
    tileWidth, tileHeight,
    result->texture->getSize().x,
    result->texture->getSize().y);
  return result;
}

Tileset MakeSimpleTileset(size_t startX, size_t startY,
                          size_t horizontalStep, size_t verticalStep,
                          size_t tileWidth, size_t tileHeight,
                          size_t imageWidth, size_t imageHeight) {
  Tileset result;

  for (size_t y = startY; (y + tileHeight) <= imageHeight; y += verticalStep) {
    for (size_t x = startX; (x + tileWidth) <= imageWidth; x += horizontalStep) {
      result.push_back(TileRect(x, y, tileWidth, tileHeight));
    }
  }

  return result;
}

} // namespace bm
