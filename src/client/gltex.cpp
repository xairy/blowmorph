#include "gltex.hpp"
#include "texture.hpp"

namespace bm {

void LoadGLTexture(GLuint id, const bm::texture<bm::rgba>& tex) {
  glBindTexture(GL_TEXTURE_RECTANGLE_ARB, id);
  
  // set bilinear filtering
  glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  
  // load texture data
  glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGBA, 
    tex.width(), tex.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, tex.ptr());
}

// Makes an OpenGL texture.
GLuint MakeGLTexture(const bm::texture<bm::rgba>& tex) {
  GLuint textureID;

  glGenTextures(1, &textureID);
  CHECK(textureID != 0);
  
  LoadGLTexture(textureID, tex);
  
  return textureID;
}

}; // namespace bm