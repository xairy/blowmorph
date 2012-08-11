#ifndef BLOWMORPH_GLMEXT_HPP_
#define BLOWMORPH_GLMEXT_HPP_

#include <glm/glm.hpp>

namespace glm {

glm::vec2 rotate(glm::vec2& vec, glm::float_t angle) {
  glm::mat2x2 mat(glm::cos(angle), -glm::sin(angle),
                  glm::sin(angle),  glm::cos(angle));
  return mat * vec;
}

};

#endif /*BLOWMORPH_GLMEXT_HPP_*/