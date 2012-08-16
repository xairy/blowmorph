#ifndef BLOWMORPH_CLIENT_TEXTURE_MANAGER_HPP_
#define BLOWMORPH_CLIENT_TEXTURE_MANAGER_HPP_

#include <map>
#include <string>

#include <SDL/SDL.h>
#include <GL/glew.h>
#include <glm/glm.hpp>

#include <base/pstdint.hpp>

namespace bm {

struct TileSetInfo {
  size_t startX;
  size_t startY; 
  size_t horizontalStep;
  size_t verticalStep;
  size_t tileWidth;
  size_t tileHeight;
  
  size_t tileCount;
  size_t tileSetWidth;
  size_t tileSetHeight;
  
  TileSetInfo(size_t startX = 0, size_t startY = 0, 
              size_t horizontalStep = 0, size_t verticalStep = 0,
              size_t tileWidth = 0, size_t tileHeight = 0) :
            startX(startX), startY(startY),
            horizontalStep(horizontalStep), verticalStep(verticalStep),
            tileWidth(tileWidth), tileHeight(tileHeight) { }
};

class Texture {
public:
  ~Texture();

  GLuint GetID() const;
  glm::uvec2 GetSize() const;
  size_t GetTileCount() const;
  glm::uvec2 GetTilePosition(size_t i) const;
  glm::uvec2 GetTileSize(size_t i) const;

private:
  Texture();

  GLuint textureID;
  glm::uvec2 size;
  TileSetInfo tileSetInfo;
  
  friend Texture* LoadOldTexture(const std::string& path,
                                 bm::uint32_t transparentColor,
                                 size_t startX, size_t startY, 
                                 size_t horizontalStep, size_t verticalStep,
                                 size_t tileWidth, size_t tileHeight);
};

// Loads an image or a tileset from a given path.
// Returns NULL on failure.
Texture* LoadOldTexture(const std::string& path,
                        bm::uint32_t transparentColor = 0xFFFFFFFF,
                        size_t startX = 0, size_t startY = 0, 
                        size_t horizontalStep = 0, size_t verticalStep = 0,
                        size_t tileWidth = 0, size_t tileHeight = 0);

} // namespace bm

#endif // BLOWMORPH_CLIENT_TEXTURE_MANAGER_HPP_
