#include <FreeImage.h>
#include "TextureManager.hpp"

namespace {

// Loads an image using FreeImage API. Selects image format based
// on file extension or image file signature.
FIBITMAP* LoadImage(const char* fileName) {
  FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
  
  // first try to guess file format from
  // its signature
  fif = FreeImage_GetFileType(fileName, 0);
  if (fif == FIF_UNKNOWN) {
    // otherwise from file name
    fif = FreeImage_GetFIFFromFilename(fileName);
  }
  
  if ((fif != FIF_UNKNOWN) && FreeImage_FIFSupportsReading(fif)) {
    FIBITMAP* dib = FreeImage_Load(fif, fileName, 0);
    return dib;
  }
  
  return NULL;
}

// Converts a bitmap to a 32-bit RGBA format.
void ConvertBitmapTo32Bits(FIBITMAP*& dib) {
  assert(dib != NULL);
  
  FIBITMAP* temp = FreeImage_ConvertTo32Bits(dib);
  FreeImage_Unload(dib);
  dib = temp;
}

// Makes certain color transparent in the image.
void MakeColorTransparent(FIBITMAP* dib, uint32_t color) {
  assert(dib != NULL);

  // must be a 32-bit bitmap
  assert(FreeImage_GetImageType(dib) == FIT_BITMAP);
  assert(FreeImage_GetBPP(dib) == 32);
  
  // color must only have RGB components
  assert((color & 0xFFFFFF) == color);

  size_t width = FreeImage_GetWidth(dib);
  size_t height = FreeImage_GetHeight(dib);
  size_t pitch = FreeImage_GetPitch(dib);
  
  unsigned char* data = reinterpret_cast<unsigned char*>(FreeImage_GetBits(dib));
  assert(data != NULL);
  
  for (size_t y = 0; y < height; y++) {
    for (size_t x = 0; x < width; x++) {
      unsigned char* row = data + y * pitch;
      unsigned char* pixel = row + x * 4;
      
      if (pixel[FI_RGBA_BLUE] == (color & 0xFF) &&
          pixel[FI_RGBA_GREEN] == ((color >> 8) & 0xFF) &&
          pixel[FI_RGBA_RED] == ((color >> 16) & 0xFF)) {
        pixel[FI_RGBA_ALPHA] = 0x00;
      }
    }
  }
}

// Makes an OpenGL texture out of a FIBITMAP bitmap.
GLuint MakeGLTexture(FIBITMAP* dib) {
  assert(dib != NULL);
  
  // must be a 32-bit bitmap
  assert(FreeImage_GetImageType(dib) == FIT_BITMAP);
  assert(FreeImage_GetBPP(dib) == 32);

  size_t width = FreeImage_GetWidth(dib);
  size_t height = FreeImage_GetHeight(dib);
  size_t pitch = FreeImage_GetPitch(dib);
  
  unsigned char* data = reinterpret_cast<unsigned char*>(FreeImage_GetBits(dib));
  assert(data != NULL);
  
  // XXX: in the following code we assume that there is no extra padding
  // at the end of each line. otherwise we'll have to use glPixelStorei
  // or something else (maybe create a buffer, copy data there and then
  // upload it via glTexImage2D).
  assert(pitch == width * 4);

  GLuint textureID;

  glGenTextures(1, &textureID);
  assert(textureID != 0);
  
  glBindTexture(GL_TEXTURE_RECTANGLE_ARB, textureID);
  
  // set bilinear filtering
  glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  
  // load texture data
  glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGBA, 
    width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, data);
  
  return textureID;
}

void PopulateTileSetInfo(FIBITMAP* dib, bm::TileSetInfo& info) {
  size_t width = FreeImage_GetWidth(dib);
  size_t height = FreeImage_GetHeight(dib);
  
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

}; // anonymous namespace

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

Texture::Texture() { }
Texture::~Texture() { 
  textureID = 0;
}

TextureManager::TextureManager() { }
TextureManager::~TextureManager() {
  UnloadAll();
}

Texture* TextureManager::Load(const std::string& path,
                              size_t startX, size_t startY, 
                              size_t horizontalStep, size_t verticalStep,
                              size_t tileWidth, size_t tileHeight,
                              ::uint32_t transparentColor) {
  if (textures.find(path) != textures.end()) {
    return textures[path];
  }
  
  FIBITMAP* dib = LoadImage(path.c_str());
  if (dib == NULL) {
    return NULL;
  }
  
  // since FreeImage has a wierd coordinate system 
  // (i.e. (0, 0) is bottom left corner), flip the image 
  // vertically to make its orientation standard
  FreeImage_FlipVertical(dib);
  
  ConvertBitmapTo32Bits(dib);
  if (dib == NULL) {
    return NULL;
  }
  
  if (transparentColor != 0xFFFFFFFF) {
    transparentColor &= 0x00FFFFFF;
    MakeColorTransparent(dib, transparentColor);
  }
  
  Texture* result = new Texture();
  result->textureID = MakeGLTexture(dib);
  
  result->tileSetInfo = TileSetInfo(startX, startY, 
                                    horizontalStep, verticalStep, 
                                    tileWidth, tileHeight);
  PopulateTileSetInfo(dib, result->tileSetInfo);
  
  FreeImage_Unload(dib);
  
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
void TextureManager::Unload(const std::string& path) {
  typedef std::map<std::string, Texture*> TexMap;
  TexMap::iterator it = textures.find(path);
  if (it != textures.end()) {
    Texture* texture = it->second;
    delete texture;
    
    textures.erase(it);
  }
}
void TextureManager::UnloadAll() {
  typedef std::map<std::string, Texture*> TexMap;
  for (TexMap::iterator it = textures.begin(); it != textures.end(); it++) {
    Texture* texture = it->second;
    delete texture;
  }
  textures.clear();
}

}; // namespace bm
