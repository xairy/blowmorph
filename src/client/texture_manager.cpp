#include "texture_manager.hpp"

#include <map>
#include <string>

#include <base/error.hpp>
#include <base/pstdint.hpp>

#include "image.hpp"

// TODO[24.7.2012 alex]: the whole TextureManager is not really needed
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

void PopulateTileSetInfo(const bm::Image& tex, bm::TileSetInfo& info) {
  size_t width = tex.Width();
  size_t height = tex.Height();
  
  if (info.tileWidth == 0) {
    info.tileWidth = width;
  }
  if (info.tileHeight == 0) {
    info.tileHeight = height;
  }
  if (info.horizontalStep == 0) {
    info.horizontalStep = info.tileWidth;
  }
  if (info.verticalStep == 0) {
    info.verticalStep = info.tileHeight;
  }
  
  info.tileSetWidth = (width - info.startX) / info.horizontalStep;
  info.tileSetHeight = (height - info.startY) / info.verticalStep;
  info.tileCount = info.tileSetWidth * info.tileSetHeight;
}

template<class U, class V>
typename std::map<U, V>::iterator FindInMap(std::map<U, V>& m, V& value) {
  for (typename std::map<U, V>::iterator it = m.begin(); it != m.end(); it++) {
    if (it->second == value) {
      return it;
    }
  }
  return m.end();
}

} // anonymous namespace

namespace bm {

GLuint Texture::GetID() const {
  assert(textureID != 0);
  return textureID;
}

size_t Texture::GetTileCount() const {
  assert(textureID != 0);
  return tileSetInfo.tileCount;
}

glm::uvec2 Texture::GetTilePosition(size_t i) const {
  assert(textureID != 0);
  assert(i < GetTileCount());
  
  size_t tileY = i / tileSetInfo.tileSetWidth;
  size_t tileX = i % tileSetInfo.tileSetWidth;
  
  return glm::uvec2(tileSetInfo.startX + tileSetInfo.horizontalStep * tileX, 
                    tileSetInfo.startY + tileSetInfo.verticalStep * tileY);
}
glm::uvec2 Texture::GetTileSize(size_t i) const {
  assert(textureID != 0);
  assert(i < GetTileCount());
  
  return glm::uvec2(tileSetInfo.tileWidth, tileSetInfo.tileHeight);
}

glm::uvec2 Texture::GetSize() const {
  CHECK(textureID != 0);
  return size;
}

Texture::Texture() { }
Texture::~Texture() {
  if (textureID != 0) {
    glDeleteTextures(1, &textureID);
  }

  textureID = 0;
}

Texture* LoadOldTexture(const std::string& path,
                        bm::uint32_t transparentColor,
                        size_t startX, size_t startY, 
                        size_t horizontalStep, size_t verticalStep,
                        size_t tileWidth, size_t tileHeight) {
  bm::Image tex;
  if (!bm::LoadRGBA(tex, path)) {
    BM_ERROR("Unable to load texture.");
    return NULL;
  }
  
  if (transparentColor != 0xFFFFFFFF) {
    transparentColor &= 0x00FFFFFF;
    MakeColorTransparent(tex, transparentColor);
  }
  
  Texture* result = new Texture();
  if(result == NULL) {
    Error::Set(Error::TYPE_MEMORY);
    return false;
  }
  result->textureID = bm::MakeGLTexture(tex);
  result->size = glm::uvec2(tex.Width(), tex.Height());
  result->tileSetInfo = TileSetInfo(startX, startY, 
                                    horizontalStep, verticalStep, 
                                    tileWidth, tileHeight);
  PopulateTileSetInfo(tex, result->tileSetInfo);
  return result;
}

} // namespace bm
