// Copyright (c) 2013 Blowmorph Team

#include "client/texture_atlas.h"

#include <map>
#include <string>

#include <SFML/Graphics.hpp>

#include "base/error.h"
#include "base/pstdint.h"

namespace bm {

TextureAtlas::TextureAtlas() : state_(STATE_FINALIZED) { }

TextureAtlas::~TextureAtlas() {
  if (state_ != STATE_FINALIZED) {
    Finalize();
  }
}

bool TextureAtlas::LoadTexture(
  const std::string& path,
  uint32_t transparent_color
) {
  CHECK(state_ == STATE_FINALIZED);

  sf::Image image;
  if (!image.loadFromFile(path)) {
    THROW_ERROR("Unable to load texture '%s'.", path.c_str());
    return false;
  }

  if (transparent_color != 0xFFFFFFFF) {
    transparent_color &= 0x00FFFFFF;
    uint8_t r = (transparent_color >> 16) & 0xFF;
    uint8_t g = (transparent_color >> 8) & 0xFF;
    uint8_t b = (transparent_color >> 0) & 0xFF;
    image.createMaskFromColor(sf::Color(r, g, b));
  }

  texture_ = new sf::Texture();
  CHECK(texture_ != NULL);
  texture_->loadFromImage(image);

  tileset_.push_back(TileRect(0, 0, image.getSize().x, image.getSize().y));
  CHECK(tileset_.size() > 0);

  state_ = STATE_INITIALIZED;
  return true;
}

bool TextureAtlas::LoadTileset(
  const std::string& path,
  uint32_t transparent_color,
  int32_t start_x, int32_t start_y,
  int32_t hor_step, int32_t ver_step,
  int32_t tile_width, int32_t tile_height
) {
  CHECK(state_ == STATE_FINALIZED);

  bool rv = LoadTexture(path, transparent_color);
  if (rv == false) {
    return false;
  }

  int32_t image_width = texture_->getSize().x;
  int32_t image_height = texture_->getSize().y;

  tileset_.clear();
  for (int32_t y = start_y; (y + tile_height) <= image_height; y += ver_step) {
    for (int32_t x = start_x; (x + tile_width) <= image_width; x += hor_step) {
      tileset_.push_back(TileRect(x, y, tile_width, tile_height));
    }
  }
  CHECK(tileset_.size() > 0);

  state_ = STATE_INITIALIZED;
  return true;
}

void TextureAtlas::Finalize() {
  CHECK(state_ == STATE_INITIALIZED);
  delete texture_;
  state_ = STATE_FINALIZED;
}

sf::Texture* TextureAtlas::GetTexture() const {
  CHECK(state_ == STATE_INITIALIZED);
  return texture_;
}

sf::Vector2i TextureAtlas::GetSize() const {
  CHECK(state_ == STATE_INITIALIZED);
  return sf::Vector2i(texture_->getSize().x, texture_->getSize().y);
}

size_t TextureAtlas::GetTileCount() const {
  CHECK(state_ == STATE_INITIALIZED);
  return tileset_.size();
}

sf::Vector2i TextureAtlas::GetTilePosition(size_t i) const {
  CHECK(state_ == STATE_INITIALIZED);
  CHECK(i < GetTileCount());
  return sf::Vector2i(tileset_[i].left, tileset_[i].top);
}

sf::Vector2i TextureAtlas::GetTileSize(size_t i) const {
  CHECK(state_ == STATE_INITIALIZED);
  CHECK(i < GetTileCount());
  return sf::Vector2i(tileset_[i].width, tileset_[i].height);
}

}  // namespace bm
