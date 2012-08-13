#include "canvas.hpp"

#include <GL/glew.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace {

void GetViewport(glm::vec2& leftTopCorner, glm::vec2& rightBottomCorner) {
  GLint viewport[4];
  glGetIntegerv(GL_VIEWPORT, viewport);
  
  float x = static_cast<float>(viewport[0]);
  float y = static_cast<float>(viewport[1]);
  float width = static_cast<float>(viewport[2]);
  float height = static_cast<float>(viewport[3]);
  
  leftTopCorner.x = x;
  leftTopCorner.y = y;
  rightBottomCorner.x = x + width;
  rightBottomCorner.y = y + height;
}

void SetProjectionMatrix(const glm::mat4x4& matrix) {
  glPushAttrib(GL_TRANSFORM_BIT);
  glMatrixMode(GL_PROJECTION);
  glLoadMatrixf(glm::value_ptr(matrix));
  glPopAttrib(); 
}

void SetModelviewMatrix(const glm::mat4x4& matrix) {
  glPushAttrib(GL_TRANSFORM_BIT);
  glMatrixMode(GL_MODELVIEW);
  glLoadMatrixf(glm::value_ptr(matrix));
  glPopAttrib(); 
}

} // anonymous namespace

namespace bm {

bool Canvas::Init() {
  return true;
}

void Canvas::SetCoordinateType(CoordinateType ct) {
  glm::vec2 leftTopCorner;
  glm::vec2 rightBottomCorner;
  GetViewport(leftTopCorner, rightBottomCorner);

  glm::mat4x4 projMatrix;
  
  if (ct == Canvas::Pixels) {
    projMatrix = glm::ortho(leftTopCorner.x,
                            rightBottomCorner.x,
                            rightBottomCorner.y,
                            leftTopCorner.y);
  } else if (ct == Canvas::PixelsFlipped) {
    projMatrix = glm::ortho(leftTopCorner.x,
                            rightBottomCorner.x,
                            leftTopCorner.y,
                            rightBottomCorner.y);
  } else if (ct == Canvas::Normalized) {
    projMatrix = glm::ortho(0, 1, 1, 0);
  } else if (ct == Canvas::NormalizedFlipped) {
    projMatrix = glm::ortho(0, 1, 0, 1);
  }
  
  SetProjectionMatrix(projMatrix);
  SetModelviewMatrix(glm::mat4x4(1));
}

void Canvas::DrawRect(const glm::vec4& clr, const glm::vec2& pos, const glm::vec2& size) {
  glBegin(GL_LINE_LOOP);
    glColor4fv(glm::value_ptr(clr));
    glVertex2f(pos.x, pos.y);
    glVertex2f(pos.x + size.x, pos.y);
    glVertex2f(pos.x + size.x, pos.y + size.y);
    glVertex2f(pos.x, pos.y + size.y);
  glEnd();
}

void Canvas::FillRect(const glm::vec4& clr, const glm::vec2& pos, const glm::vec2& size) {
  glBindTexture(GL_TEXTURE_RECTANGLE_ARB, 0);
  glBindTexture(GL_TEXTURE_2D, 0);

  glBegin(GL_QUADS);
    glColor4fv(glm::value_ptr(clr));
    glVertex2f(pos.x, pos.y);
    glVertex2f(pos.x + size.x, pos.y);
    glVertex2f(pos.x + size.x, pos.y + size.y);
    glVertex2f(pos.x, pos.y + size.y);
  glEnd();
}

} // namespace bm