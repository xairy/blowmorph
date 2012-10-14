#include "texture_atlas.hpp"

#include <map>
#include <string>

#include <base/error.hpp>
#include <base/pstdint.hpp>

#include "image.hpp"
#include "texture.hpp"

// TODO[24.7.2012 alex]: better think of something like "ResourceManager"

namespace {

void MakeColorTransparent(bm::Image& tex, uint32_t transparentColor) {
  CHECK((transparentColor & 0xFF000000) == 0);
  
  bm::rgb clr = bm::rgb((transparentColor >> 16) & 0xFF,
                        (transparentColor >>  8) & 0xFF,
                        (transparentColor >>  0) & 0xFF);
                        
  for (size_t y = 0; y < tex.Height(); y++) {
    for (size_t x = 0; x < tex.Width(); x++) {
      if (tex(x, y).r == clr.r && tex(x, y).g == clr.g && tex(x, y).b == clr.b) {
        tex(x, y).a = 0;
      }
    }
  }
}

} // anonymous namespace

namespace bm {

Texture* TextureAtlas::GetTexture() const {
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
  return glm::vec2(texture->Width(), texture->Height());
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
  bm::Image image;
  if (!bm::LoadRGBA(image, path)) {
    BM_ERROR("Unable to load texture.");
    return NULL;
  }
  
  if (transparentColor != 0xFFFFFFFF) {
    transparentColor &= 0x00FFFFFF;
    MakeColorTransparent(image, transparentColor);
  }
  
  TextureAtlas* result = new TextureAtlas();
  CHECK(result != NULL);
  
  result->texture = new Texture();
  CHECK(result->texture != NULL);

  if (!result->texture->Create(image.Width(), image.Height())) {
    delete result->texture;
    delete result;
    return NULL;
  }
  if (!result->texture->Update(&image)) {
    delete result->texture;
    delete result;
    return NULL;
  }
  
  result->tileset.push_back(TileRect(0, 0, image.Width(), image.Height()));
  
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
    result->texture->Width(),
    result->texture->Height());
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
