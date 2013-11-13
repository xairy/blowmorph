// Copyright (c) 2013 Blowmorph Team

#include "client/texture_atlas.h"

#include <map>
#include <string>

#include <SFML/Graphics.hpp>

#include "base/error.h"
#include "base/pstdint.h"

// TODO(alex): better think of something like "ResourceManager"

namespace bm {

sf::Texture* TextureAtlas::GetTexture() const {
  return texture;
}

size_t TextureAtlas::GetTileCount() const {
  CHECK(texture != 0);
  return tileset.size();
}

sf::Vector2i TextureAtlas::GetTilePosition(size_t i) const {
  CHECK(texture != 0);
  CHECK(i < GetTileCount());

  return sf::Vector2i(tileset[i].left, tileset[i].top);
}

sf::Vector2i TextureAtlas::GetTileSize(size_t i) const {
  CHECK(texture != NULL);
  CHECK(i < GetTileCount());

  return sf::Vector2i(tileset[i].width, tileset[i].height);
}

sf::Vector2i TextureAtlas::GetSize() const {
  CHECK(texture != NULL);
  return sf::Vector2i(texture->getSize().x, texture->getSize().y);
}

TextureAtlas::TextureAtlas() { }
TextureAtlas::~TextureAtlas() {
  if (texture != NULL) {
    delete texture;
    texture = NULL;
  }
}

TextureAtlas* LoadTexture(
  const std::string& path,
  uint32_t transparent_color
) {
  sf::Image image;
  if (!image.loadFromFile(path)) {
    BM_ERROR("Unable to load texture.");
    return NULL;
  }

  if (transparent_color != 0xFFFFFFFF) {
    transparent_color &= 0x00FFFFFF;
    uint8_t r = (transparent_color >> 16) & 0xFF;
    uint8_t g = (transparent_color >> 8) & 0xFF;
    uint8_t b = (transparent_color >> 0) & 0xFF;
    image.createMaskFromColor(sf::Color(r, g, b));
  }

  TextureAtlas* result = new TextureAtlas();
  CHECK(result != NULL);

  result->texture = new sf::Texture();
  CHECK(result->texture != NULL);
  result->texture->loadFromImage(image);

  result->tileset.push_back(
    TileRect(0, 0, image.getSize().x, image.getSize().y));

  CHECK(result->tileset.size() > 0);
  return result;
}

TextureAtlas* LoadTileset(
  const std::string& path,
  uint32_t transparent_color,
  int32_t start_x, int32_t start_y,
  int32_t horizontal_step, int32_t vertical_step,
  int32_t tile_width, int32_t tile_height
) {
  TextureAtlas* result = LoadTexture(path, transparent_color);
  if (result == NULL) {
    return NULL;
  }

  result->tileset.clear();
  result->tileset = MakeSimpleTileset(
    start_x, start_y,
    horizontal_step, vertical_step,
    tile_width, tile_height,
    result->texture->getSize().x,
    result->texture->getSize().y);
  CHECK(result->tileset.size() > 0);
  return result;
}

TileSet MakeSimpleTileset(
  int32_t start_x, int32_t start_y,
  int32_t hor_step, int32_t ver_step,
  int32_t tile_width, int32_t tile_height,
  int32_t image_width, int32_t image_height
) {
  TileSet result;

  for (int32_t y = start_y; (y + tile_height) <= image_height; y += ver_step) {
    for (int32_t x = start_x; (x + tile_width) <= image_width; x += hor_step) {
      result.push_back(TileRect(x, y, tile_width, tile_height));
    }
  }

  return result;
}

}  // namespace bm
