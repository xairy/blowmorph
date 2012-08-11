#include "texture.hpp"

#include <FreeImage.h>

namespace {

// Loads an image using FreeImage API. Selects image format based
// on file extension or image file signature.
// Returns 'NULL' on error.
FIBITMAP* LoadImage(const char* fileName) {
  CHECK(fileName != NULL);

  FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
  
  // first try to guess file format from its signature
  fif = FreeImage_GetFileType(fileName, 0);
  if (fif == FIF_UNKNOWN) {
    // otherwise from file name
    fif = FreeImage_GetFIFFromFilename(fileName);
  }
  
  if ((fif != FIF_UNKNOWN) && FreeImage_FIFSupportsReading(fif)) {
    FIBITMAP* dib = FreeImage_Load(fif, fileName, 0);
    return dib;
  }
  
  return NULL;
}

// Saves an image using FreeImage API. Selects image format based
// on file extension.
// Return 'true' on success, return 'false' on failure.
bool SaveImage(const char* fileName, FIBITMAP* src) {
  CHECK(fileName != NULL);
  CHECK(src != NULL);

  FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
  fif = FreeImage_GetFIFFromFilename(fileName);
  
  if ((fif != FIF_UNKNOWN) && FreeImage_FIFSupportsWriting(fif)) {
    bool rv = FreeImage_Save(fif, src, fileName, 0);
    return rv;
  }
  return false;
}

// Converts a bitmap to a 32-bit RGBA format.
void ConvertBitmapTo32Bits(FIBITMAP** dib) {
  CHECK(dib != NULL);
  CHECK(*dib != NULL);
  
  // if bitmap is already converted, do nothing
  if(FreeImage_GetImageType(*dib) == FIT_BITMAP && FreeImage_GetBPP(*dib) == 32) {
    return;
  }
  
  FIBITMAP* temp = FreeImage_ConvertTo32Bits(*dib);
  FreeImage_Unload(*dib);
  *dib = temp;
}

void BitmapToTex(bm::texture& dst, FIBITMAP* src) {
  CHECK(src != NULL);
  
  // must be a 32-bit bitmap
  CHECK(FreeImage_GetImageType(src) == FIT_BITMAP);
  CHECK(FreeImage_GetBPP(src) == 32);

  size_t width = FreeImage_GetWidth(src);
  size_t height = FreeImage_GetHeight(src);
  size_t pitch = FreeImage_GetPitch(src);
  
  dst.SetSize(width, height);
  
  unsigned char* data = reinterpret_cast<unsigned char*>(FreeImage_GetBits(src));
  CHECK(data != NULL);
  
  for (size_t y = 0; y < height; y++) {
    for (size_t x = 0; x < width; x++) {
      unsigned char* row = data + y * pitch;
      unsigned char* pixel = row + x * 4;
      
      dst(x, y).r = pixel[FI_RGBA_RED];
      dst(x, y).g = pixel[FI_RGBA_GREEN];
      dst(x, y).b = pixel[FI_RGBA_BLUE];
      dst(x, y).a = pixel[FI_RGBA_ALPHA];
    }
  }
}

void TexToBitmap(FIBITMAP* dst, const bm::texture& src) {
  CHECK(dst != NULL);
  
  // must be a 32-bit bitmap
  CHECK(FreeImage_GetImageType(dst) == FIT_BITMAP);
  CHECK(FreeImage_GetBPP(dst) == 32);
  
  // sizes must be equal
  CHECK(FreeImage_GetWidth(dst) == src.Width());
  CHECK(FreeImage_GetHeight(dst) == src.Height());

  size_t width = FreeImage_GetWidth(dst);
  size_t height = FreeImage_GetHeight(dst);
  size_t pitch = FreeImage_GetPitch(dst);
  
  unsigned char* data = reinterpret_cast<unsigned char*>(FreeImage_GetBits(dst));
  CHECK(data != NULL);
  
  for (size_t y = 0; y < height; y++) {
    for (size_t x = 0; x < width; x++) {
      unsigned char* row = data + y * pitch;
      unsigned char* pixel = row + x * 4;
      
      pixel[FI_RGBA_RED] = src(x, y).r;
      pixel[FI_RGBA_GREEN] = src(x, y).g;
      pixel[FI_RGBA_BLUE] = src(x, y).b;
      pixel[FI_RGBA_ALPHA] = src(x, y).a;
    }
  }
}

} // anonymous namespace

namespace bm {

bool LoadRGBA(texture& tex, const std::string& path) {
  FIBITMAP* dib = LoadImage(path.c_str());
  if (dib == NULL) {
    return false;
  }
  
  // since FreeImage has a wierd coordinate system 
  // (i.e. (0, 0) is bottom left corner), flip the image 
  // vertically to make its orientation standard
  FreeImage_FlipVertical(dib);
  
  ConvertBitmapTo32Bits(&dib);
  if (dib == NULL) {
    return false;
  }
  
  BitmapToTex(tex, dib);
  
  FreeImage_Unload(dib);
  
  return true;
}

bool SaveRGBA(const std::string& path, const texture& src) {
  // XXX[29.7.2012 alex]: casts size_t to int
  FIBITMAP* dib = FreeImage_Allocate(static_cast<int>(src.Width()), static_cast<int>(src.Height()), 32);
  if (dib == NULL) {
    return false;
  }
  
  TexToBitmap(dib, src);
  
  if (!SaveImage(path.c_str(), dib)) {
    FreeImage_Unload(dib);
    return false;
  }
  
  FreeImage_Unload(dib);
  
  return true;
}

void LoadGLTexture(GLuint id, const bm::texture& tex) {
  glBindTexture(GL_TEXTURE_RECTANGLE_ARB, id);
  
  // set bilinear filtering
  glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  
  // load texture data
  // XXX[29.7.2012 alex]: casts size_t to GLsizei
  glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGBA, 
    static_cast<GLsizei>(tex.Width()), static_cast<GLsizei>(tex.Height()), 
    0, GL_RGBA, GL_UNSIGNED_BYTE, tex.Data());
}

// Makes an OpenGL texture.
GLuint MakeGLTexture(const bm::texture& tex) {
  GLuint textureID;

  glGenTextures(1, &textureID);
  CHECK(textureID != 0);
  
  LoadGLTexture(textureID, tex);
  
  return textureID;
}

} // namespace bm
