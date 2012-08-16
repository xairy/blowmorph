// XXX[12.8.2012 alex]: better include order?
#define _USE_MATH_DEFINES
#include "sprite.hpp"

#include <cmath>

#include <GL/glew.h>
#include <glm/glm.hpp>

#include <base/macros.hpp>

#include "texture_atlas.hpp"

namespace bm {

Sprite::Sprite() { }
Sprite::~Sprite() { }

bool Sprite::Init(bm::TextureAtlas* texture, size_t tile) {
  CHECK(texture != NULL);
  CHECK(tile < texture->GetTileCount());

  this->texture = texture;
  this->tile = tile;  
  
  zIndex = 0;
  position = glm::vec2(0, 0);
  pivot = glm::vec2(0, 0);
  scale = glm::vec2(1, 1);
  angle = 0;
  
  return true;
}

void Sprite::Render() {  
  glColor3f(1.0f, 1.0f, 1.0f);
  glBindTexture(GL_TEXTURE_RECTANGLE_ARB, texture->GetID());
  
  glPushMatrix();
  //glLoadIdentity();
  glTranslatef(position.x, position.y, 0);
  glScalef(scale.x, scale.y, 1.0);
  glRotatef(static_cast<GLfloat>(angle / M_PI * 180.0f), 0.0, 0.0, 1.0);
  
  glm::uvec2 size = texture->GetTileSize(tile);
  glm::uvec2 tilePos = texture->GetTilePosition(tile);
  
  glBegin(GL_QUADS);
    glTexCoord2i(tilePos.x, tilePos.y);
    glVertex3f(-pivot.x * size.x, -pivot.y * size.y, zIndex);
    
    glTexCoord2i(tilePos.x, tilePos.y + size.y);
    glVertex3f(-pivot.x * size.x, (1 - pivot.y) * size.y, zIndex);
    
    glTexCoord2i(tilePos.x + size.x, tilePos.y + size.y);
    glVertex3f((1 - pivot.x) * size.x, (1 - pivot.y) * size.y, zIndex);
    
    glTexCoord2i(tilePos.x + size.x, tilePos.y);
    glVertex3f((1 - pivot.x) * size.x, -pivot.y * size.y, zIndex);
  glEnd();
  
  glPopMatrix();
}

void Sprite::SetZIndex(glm::float_t value) {
  zIndex = value;
}
glm::float_t Sprite::GetZIndex() const {
  return zIndex;
}

void Sprite::SetPosition(const glm::vec2& value) {
  position = value;
}
glm::vec2 Sprite::GetPosition() const {
  return position;
}

void Sprite::SetPivot(const glm::vec2& value) {
  // XXX[12.08.2012 xairy]: GetSize -> GetTileSize ?
  glm::uvec2 size = texture->GetTileSize(tile);
  glm::vec2 pivotChange = value - pivot;
  pivot = value;
  position += glm::vec2(pivotChange.x * size.x, pivotChange.y * size.y);
}
glm::vec2 Sprite::GetPivot() const {
  return pivot;
}

void Sprite::SetScale(const glm::vec2& value) {
  scale = value;
}
glm::vec2 Sprite::GetScale() const {
  return scale;
}

void Sprite::SetAngle(glm::float_t value) {
  angle = value;
}
glm::float_t Sprite::GetAngle() const {
  return angle;
}

glm::uvec2 Sprite::GetSize() const {
  return texture->GetSize();
}

void Sprite::Rotate(glm::float_t value) {
  angle += value;
}
void Sprite::Move(const glm::vec2& value) {
  position += value;
}

void Sprite::SetTile(size_t tile) {
  CHECK(tile < texture->GetTileCount());
  this->tile = tile;
}
size_t Sprite::GetTile() const {
  return tile;
}

} // namespace bm
