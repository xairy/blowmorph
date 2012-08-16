#include "texture_atlas.hpp"

#include <map>
#include <string>

#include <base/error.hpp>
#include <base/pstdint.hpp>

#include "image.hpp"

// TODO[24.7.2012 alex]: better think of something like "ResourceManager"

namespace {

void MakeColorTransparent(bm::Image& tex, ::uint32_t transparentColor) {
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

GLuint TextureAtlas::GetID() const {
  assert(textureID != 0);
  return textureID;
}

size_t TextureAtlas::GetTileCount() const {
  assert(textureID != 0);
  return tileSetInfo.size();
}

glm::uvec2 TextureAtlas::GetTilePosition(size_t i) const {
  assert(textureID != 0);
  assert(i < GetTileCount());
  
  return glm::uvec2(tileSetInfo[i].x, tileSetInfo[i].y);
}
glm::uvec2 TextureAtlas::GetTileSize(size_t i) const {
  assert(textureID != 0);
  assert(i < GetTileCount());
  
  return glm::uvec2(tileSetInfo[i].width, tileSetInfo[i].height);
}

glm::uvec2 TextureAtlas::GetSize() const {
  CHECK(textureID != 0);
  return size;
}

TextureAtlas::TextureAtlas() { }
TextureAtlas::~TextureAtlas() {
  if (textureID != 0) {
    glDeleteTextures(1, &textureID);
  }

  textureID = 0;
}

TextureAtlas* LoadOldTexture(const std::string& path,
                        bm::uint32_t transparentColor) {
  bm::Image tex;
  if (!bm::LoadRGBA(tex, path)) {
    BM_ERROR("Unable to load texture.");
    return NULL;
  }
  
  if (transparentColor != 0xFFFFFFFF) {
    transparentColor &= 0x00FFFFFF;
    MakeColorTransparent(tex, transparentColor);
  }
  
  TextureAtlas* result = new TextureAtlas();
  if(result == NULL) {
    Error::Set(Error::TYPE_MEMORY);
    return false;
  }
  result->textureID = bm::MakeGLTexture(tex);
  result->size = glm::uvec2(tex.Width(), tex.Height());
  result->tileSetInfo.push_back(TileRect(0, 0, tex.Width(), tex.Height()));
  return result;
}

TextureAtlas* LoadTileset(const std::string& path,
                     bm::uint32_t transparentColor,
                     size_t startX, size_t startY, 
                     size_t horizontalStep, size_t verticalStep,
                     size_t tileWidth, size_t tileHeight) {
  TextureAtlas* result = LoadOldTexture(path, transparentColor);
  if(result == NULL) {
    Error::Set(Error::TYPE_MEMORY);
    return false;
  }
  result->tileSetInfo.clear();
  result->tileSetInfo = MakeSimpleTileset(startX, startY, 
    horizontalStep, verticalStep, 
    tileWidth, tileHeight,
    result->size.x,
    result->size.y);
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
