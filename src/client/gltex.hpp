#ifndef BLOWMORPH_GLTEX_HPP_
#define BLOWMORPH_GLTEX_HPP_

#include <GL/glew.h>
#include "texture.hpp"

namespace bm {

void LoadGLTexture(GLuint id, const bm::texture<bm::rgba>& tex);
GLuint MakeGLTexture(const bm::texture<bm::rgba>& tex);

}; // namespace bm

#endif//BLOWMORPH_GLTEX_HPP_