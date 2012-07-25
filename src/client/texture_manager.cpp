#include "texture_manager.hpp"
#include "texture.hpp"
#include "gltex.hpp"

// TODO[24.7.2012 alex]: the whole TextureManager is not really needed
// TODO[24.7.2012 alex]: better think of something like "ResourceManager"

namespace {

void MakeColorTransparent(bm::texture<bm::rgba>& tex, ::uint32_t transparentColor) {
  CHECK((transparentColor & 0xFF000000) == 0);
  
  bm::rgb clr = bm::rgb((transparentColor >> 16) & 0xFF,
                        (transparentColor >>  8) & 0xFF,
                        (transparentColor >>  0) & 0xFF);
                        
  for (size_t y = 0; y < tex.height(); y++) {
    for (size_t x = 0; x < tex.width(); x++) {
      if (tex(x, y).r == clr.r && tex(x, y).g == clr.g && tex(x, y).b == clr.b) {
        tex(x, y).a = 0;
      }
    }
  }
}

//void PopulateTileSetInfo(const bm::texture<bm::rgba>& tex, bm::TileSetInfo& info) {
//  size_t width = tex.width();
//  size_t height = tex.height();
//  
//  if (info.tileWidth == 0) {
//    info.tileWidth = width;
//  }
//  if (info.tileHeight == 0) {
//    info.tileHeight = height;
//  }
//  if (info.horizontalStep == 0) {
//    info.horizontalStep = info.tileWidth;
//  }
//  if (info.verticalStep == 0) {
//    info.verticalStep = info.tileHeight;
//  }
//  
//  info.tileSetWidth = (width - info.startX) / info.horizontalStep;
//  info.tileSetHeight = (height - info.startY) / info.verticalStep;
//  info.tileCount = info.tileSetWidth * info.tileSetHeight;
//}

template<class U, class V>
typename std::map<U, V>::iterator FindInMap(std::map<U, V>& m, V& value) {
  for (typename std::map<U, V>::iterator it = m.begin(); it != m.end(); it++) {
    if (it->second == value) {
      return it;
    }
  }
  return m.end();
}

}; // anonymous namespace

namespace bm {

GLuint Texture::GetID() const {
  assert(textureID != 0);
  return textureID;
}

//size_t Texture::GetTileCount() const {
//  assert(textureID != 0);
//  return tileSetInfo.tileCount;
//}
//
//glm::uvec2 Texture::GetTilePosition(size_t i) const {
//  assert(textureID != 0);
//  assert(i < GetTileCount());
//  
//  size_t tileY = i / tileSetInfo.tileSetWidth;
//  size_t tileX = i % tileSetInfo.tileSetWidth;
//  
//  return glm::uvec2(tileSetInfo.startX + tileSetInfo.horizontalStep * tileX, 
//                    tileSetInfo.startY + tileSetInfo.verticalStep * tileY);
//}
//glm::uvec2 Texture::GetTileSize(size_t i) const {
//  assert(textureID != 0);
//  assert(i < GetTileCount());
//  
//  return glm::uvec2(tileSetInfo.tileWidth, tileSetInfo.tileHeight);
//}

glm::uvec2 Texture::GetSize() const {
  CHECK(textureID != 0);
  return size;
}

Texture::Texture() { }
Texture::~Texture() { 
  textureID = 0;
}

TextureManager::TextureManager() { }
TextureManager::~TextureManager() {
  UnloadAll();
}

Texture* TextureManager::Load(const std::string& path,
                              /*size_t startX, size_t startY, 
                              size_t horizontalStep, size_t verticalStep,
                              size_t tileWidth, size_t tileHeight,*/
                              bm::uint32_t transparentColor) {
  if (textures.find(path) != textures.end()) {
    return textures[path];
  }
  
  bm::texture<bm::rgba> tex;
  bm::LoadRGBA(tex, path);
  
  if (transparentColor != 0xFFFFFFFF) {
    transparentColor &= 0x00FFFFFF;
    MakeColorTransparent(tex, transparentColor);
  }
  
  Texture* result = new Texture();
  result->textureID = bm::MakeGLTexture(tex);
  result->size = glm::uvec2(tex.width(), tex.height());
  /*result->tileSetInfo = TileSetInfo(startX, startY, 
                                    horizontalStep, verticalStep, 
                                    tileWidth, tileHeight);
  PopulateTileSetInfo(tex, result->tileSetInfo);*/
  textures[path] = result;
  return result;
}

void TextureManager::Unload(Texture* texture) {
  typedef std::map<std::string, Texture*> TexMap;
  TexMap::iterator it = FindInMap(textures, texture);
  if (it != textures.end()) {
    Texture* texture = it->second;
    delete texture;
    
    textures.erase(it);
  }
}
//void TextureManager::Unload(const std::string& path) {
//  typedef std::map<std::string, Texture*> TexMap;
//  TexMap::iterator it = textures.find(path);
//  if (it != textures.end()) {
//    Texture* texture = it->second;
//    delete texture;
//    
//    textures.erase(it);
//  }
//}
void TextureManager::UnloadAll() {
  typedef std::map<std::string, Texture*> TexMap;
  for (TexMap::iterator it = textures.begin(); it != textures.end(); it++) {
    Texture* texture = it->second;
    delete texture;
  }
  textures.clear();
}

}; // namespace bm
