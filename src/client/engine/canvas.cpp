#include "canvas.hpp"

#include <base/macros.hpp>
#include "texture_atlas.hpp"
#include "texture.hpp"

#include <GL/glew.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace {

static float fround(float f) {
  return ::floorf(f + 0.5f);
}

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

glm::vec2 Canvas::GetSize() {
  glm::vec2 leftTopCorner;
  glm::vec2 rightBottomCorner;
  GetViewport(leftTopCorner, rightBottomCorner);
  glm::vec2 result = rightBottomCorner - leftTopCorner;
  return result;
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

void Canvas::SetTransform(const glm::mat3x3& m) {
  // Make a matrix corresponding to the same transform:
  //  x' = m11 x + m12 y + m13
  //  y' = m21 x + m22 y + m23
  //  w' = m31 x + m32 y + m33
  // with an additional equation for z':
  //  z' = z
  glm::mat4x4 mvm;
  mvm[0] = glm::vec4(m[0][0], m[0][1], 0, m[0][2]);
  mvm[1] = glm::vec4(m[1][0], m[1][1], 0, m[1][2]);
  mvm[2] = glm::vec4(0,       0,       1,       0);
  mvm[3] = glm::vec4(m[2][0], m[2][1], 0, m[2][2]);
  
  SetModelviewMatrix(mvm);
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

void Canvas::DrawCircle(const glm::vec4& clr, const glm::vec2& pos, float radius, size_t steps) {
  glBegin(GL_LINE_LOOP);
    glColor4fv(glm::value_ptr(clr));
    for (size_t i = 0; i < steps; i++) {
      double angle = 2 * M_PI / steps * i;
      glVertex2d(pos.x + radius * ::cos(angle), pos.y + radius * ::sin(angle));
    }
  glEnd();
}
void Canvas::FillCircle(const glm::vec4& clr, const glm::vec2& pos, float radius, size_t steps) {
  glBindTexture(GL_TEXTURE_RECTANGLE_ARB, 0);
  glBindTexture(GL_TEXTURE_2D, 0);
  
  glBegin(GL_TRIANGLE_FAN);
    glColor4fv(glm::value_ptr(clr));
    glVertex2d(pos.x, pos.y);
    for (size_t i = 0; i <= steps; i++) {
      double angle = 2 * M_PI / steps * i;
      glVertex2d(pos.x + radius * ::cos(angle), pos.y + radius * ::sin(angle));
    }
  glEnd();
}

void Canvas::DrawTexturedQuad(const glm::vec2& pos, const glm::vec2& pivot, glm::float_t zIndex, 
                              TextureAtlas* texture, size_t tile) {
  assert(texture != NULL);
  assert(tile < texture->GetTileCount());
  
  glColor3f(1.0f, 1.0f, 1.0f);

  texture->GetTexture()->Bind(bm::Texture::Pixels);

  glPushMatrix();
  //glLoadIdentity();
  glTranslatef(::fround(pos.x), ::fround(pos.y), 0);
  //glScalef(scale.x, scale.y, 1.0);
  //glRotatef(static_cast<GLfloat>(angle / M_PI * 180.0f), 0.0, 0.0, 1.0);

  glm::vec2 size = texture->GetTileSize(tile);
  glm::vec2 tilePos = texture->GetTilePosition(tile);

  glBegin(GL_QUADS);
  glTexCoord2f(tilePos.x + 0.5f, tilePos.y + 0.5f);
  glVertex3f(-pivot.x * size.x, -pivot.y * size.y, zIndex);

  glTexCoord2f(tilePos.x + 0.5f, tilePos.y + size.y - 0.5f);
  glVertex3f(-pivot.x * size.x, (1 - pivot.y) * size.y, zIndex);

  glTexCoord2f(tilePos.x + size.x - 0.5f, tilePos.y + size.y - 0.5f);
  glVertex3f((1 - pivot.x) * size.x, (1 - pivot.y) * size.y, zIndex);

  glTexCoord2f(tilePos.x + size.x - 0.5f, tilePos.y + 0.5f);
  glVertex3f((1 - pivot.x) * size.x, -pivot.y * size.y, zIndex);
  glEnd();

  glPopMatrix();
}

} // namespace bm
