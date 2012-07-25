#ifndef BLOWMORPH_TEXTUREMANAGER_HPP_
#define BLOWMORPH_TEXTUREMANAGER_HPP_

#include <string>
#include <map>

#include <SDL/SDL.h>
#include <GL/glew.h>
#include <glm/glm.hpp>

#include <base/pstdint.hpp>

namespace bm {

class Texture;
class TextureManager;

//struct TileSetInfo {
//  size_t startX;
//  size_t startY; 
//  size_t horizontalStep;
//  size_t verticalStep;
//  size_t tileWidth;
//  size_t tileHeight;
//  
//  size_t tileCount;
//  size_t tileSetWidth;
//  size_t tileSetHeight;
//  
//  TileSetInfo(size_t startX = 0, size_t startY = 0, 
//              size_t horizontalStep = 0, size_t verticalStep = 0,
//              size_t tileWidth = 0, size_t tileHeight = 0) :
//            startX(startX), startY(startY),
//            horizontalStep(horizontalStep), verticalStep(verticalStep),
//            tileWidth(tileWidth), tileHeight(tileHeight) { }
//};

class Texture {
public:
  GLuint GetID() const;
  glm::uvec2 GetSize() const;
  //size_t GetTileCount() const;
  //glm::uvec2 GetTilePosition(size_t i) const;
  //glm::uvec2 GetTileSize(size_t i) const;

private:
  Texture();
  ~Texture();

  GLuint textureID;
  glm::uvec2 size;
  //TileSetInfo tileSetInfo;
  
  friend class TextureManager;
};

class TextureManager {
public:
  TextureManager();
  ~TextureManager();
  
  // Loads an image or a tileset from a given path.
  // FIXME: returns NULL on failure?
  Texture* Load(const std::string& path,
                //size_t startX = 0, size_t startY = 0, 
                //size_t horizontalStep = 0, size_t verticalStep = 0,
                //size_t tileWidth = 0, size_t tileHeight = 0,
                bm::uint32_t transparentColor = 0xFFFFFFFF);

  // Unloads the given texture.
  void Unload(Texture* texture);

  // FIXME: можно ли Load'ом загрузить одну и ту же текстуру дважды? Тогда как
  // поведёт себя Unload по пути? Вам правда нужно обе версии метода?
  //void Unload(const std::string& path);
  
  // Unloads all previously loaded textures. 
  // Note that all Texture* pointers become invalidated after unloading.
  void UnloadAll();

private:
  std::map<std::string, Texture*> textures;
};

}; // namespace bm

#endif /*BLOWMORPH_TEXTUREMANAGER_HPP_*/
