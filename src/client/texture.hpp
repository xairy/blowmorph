#ifndef BLOWMORPH_CLIENT_TEXTURE_HPP_
#define BLOWMORPH_CLIENT_TEXTURE_HPP_

#include <cstring>

#include <string>

#include <GL/glew.h>
#include <glm/glm.hpp>

#include <base/pstdint.hpp>
#include <base/macros.hpp>

#include "array.hpp"

namespace bm {

typedef glm::detail::tvec3<bm::uint8_t> rgb;
typedef glm::detail::tvec4<bm::uint8_t> rgba;
typedef glm::vec3 rgbf;
typedef glm::vec4 rgbaf;

typedef Array2<rgba> Image;

SCHECK(sizeof(rgb) == 3);
SCHECK(sizeof(rgba) == 4);

bool LoadRGBA(Image& dst, const std::string& path);
bool SaveRGBA(const std::string& path, const Image& src);

void LoadGLTexture(GLuint id, const bm::Image& tex);
GLuint MakeGLTexture(const bm::Image& tex);

} // namespace bm

#endif // BLOWMORPH_CLIENT_TEXTURE_HPP_
