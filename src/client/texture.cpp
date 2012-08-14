#include "texture.hpp"

#include <base/error.hpp>

#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>

namespace {

// Returns the nearest power of two which is bigger than
// the value given.
size_t NearestPowerOfTwo(size_t value) {
  // Check for possible overflow.
  size_t MAX_BITS = sizeof(size_t) * 8 - 1;
  CHECK(value < size_t(1 << MAX_BITS));
  
  size_t result = 1;
  while (result < value) {
    result *= 2;
  }
  return result;
}

// Returns a minimum valid texture size greater or equal to
// the original size.
size_t MinValidSize(size_t size) {
  if (GLEW_ARB_texture_non_power_of_two) {
    return size; 
  } else {
    return NearestPowerOfTwo(size);
  }
}

// Returns the maximum supported texture size.
size_t GetMaximumSize() {
  GLint size;
  glGetIntegerv(GL_MAX_TEXTURE_SIZE, &size);
  
  SCHECK(sizeof(size_t) >= sizeof(GLint));
  return static_cast<size_t>(size);
}

void SetTextureMatrix(const glm::mat4x4& matrix) {
  glPushAttrib(GL_TRANSFORM_BIT);
  glMatrixMode(GL_TEXTURE);
  glLoadMatrixf(glm::value_ptr(matrix));
  glPopAttrib(); 
}

} // anonymous namespace

namespace bm {

NewTexture::NewTexture() : _texture_id(0), _width(0), _height(0), _actual_width(0), _actual_height(0) { }
NewTexture::~NewTexture() {
  if (_texture_id != 0) {
    glDeleteTextures(1, &_texture_id);
    _texture_id = 0;
  }
}

bool NewTexture::Create(size_t width, size_t height) {
  CHECK(width > 0 && height > 0);
  
  _width = width;
  _height = height;
  _actual_width = MinValidSize(width);
  _actual_height = MinValidSize(height);
  
  if (_actual_width > GetMaximumSize() || _actual_height > GetMaximumSize()) {
    BM_ERROR("NewTexture size is too big.");
    return false;
  }
  
  if (!_texture_id) {
    glGenTextures(1, &_texture_id);
    if (!_texture_id) {
      BM_ERROR("Could not create texture.");
      return false;
    }
  }
  
  glBindTexture(GL_TEXTURE_2D, _texture_id);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, _actual_width, _actual_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, _flags.repeated ? GL_REPEAT : GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, _flags.repeated ? GL_REPEAT : GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, _flags.smooth ? GL_LINEAR : GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, _flags.smooth ? GL_LINEAR : GL_NEAREST);
  
  return true;
}

bool NewTexture::Update(const bm::Image* image) {
  CHECK(_texture_id != 0);
  CHECK(_width == image->Width() && _height == image->Height());
  
  glBindTexture(GL_TEXTURE_2D, _texture_id);
  
  SCHECK(sizeof(GLsizei) >= sizeof(image->Width()));
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 
                  static_cast<GLsizei>(image->Width()),
                  static_cast<GLsizei>(image->Height()),
                  GL_RGBA, GL_UNSIGNED_BYTE, image->Data());
    
  return true;
}

bool NewTexture::Bind(CoordinateType ct) {
  CHECK(_texture_id != 0);
  CHECK(ct == NewTexture::Normalized || ct == NewTexture::Pixels);
  
  glBindTexture(GL_TEXTURE_2D, _texture_id);
  
  glm::mat4x4 matrix(1);  
  
  if (ct == NewTexture::Pixels) {
    matrix[0][0] /= _width;
    matrix[1][1] /= _height;
  }
  
  if (_flags.flipped) {
    matrix[1][1] = - matrix[1][1];
    matrix[3][1] = 1;
  }
  
  SetTextureMatrix(matrix);
  
  return true;
}

void NewTexture::SetFlags(bool smooth, bool repeated, bool flipped) {
  _flags.smooth = smooth ? 1 : 0;
  _flags.repeated = repeated ? 1 : 0;
  _flags.flipped = flipped ? 1 : 0;
}

bool NewTexture::IsValid() const {
  return _texture_id != 0;
}
bool NewTexture::IsSmooth() const {
  return _flags.smooth != 0;
}
bool NewTexture::IsRepeated() const {
  return _flags.repeated != 0;
}
bool NewTexture::IsFlipped() const {
  return _flags.flipped != 0;
}

size_t NewTexture::Width() const {
  return IsValid() ? _width : 0;
}
size_t NewTexture::Height() const {
  return IsValid() ? _height : 0;
}

} // namespace bm