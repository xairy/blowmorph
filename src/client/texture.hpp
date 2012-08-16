#ifndef BLOWMORPH_CLIENT_TEXTURE_HPP_
#define BLOWMORPH_CLIENT_TEXTURE_HPP_

#include <base/macros.hpp>
#include "image.hpp"

namespace bm {

class Texture {
public:
  enum CoordinateType {
    Normalized,
    Pixels
  };

  Texture();
  ~Texture();

  // Creates an empty texture with given width and height.
  // Width and height must be positive.
  bool Create(size_t width, size_t height);
  
  void Dispose();
  
  bool Update(const void* rgbaData);
  
  // Updates the texture from an image.
  bool Update(const bm::Image* image);
  
  bool Bind(CoordinateType ct);
  
  void SetFlags(bool smooth, bool repeated, bool flipped);
  
  bool IsValid() const;
  bool IsSmooth() const;
  bool IsRepeated() const;
  bool IsFlipped() const;
  
  size_t Width() const;
  size_t Height() const;

private:
  DISALLOW_COPY_AND_ASSIGN(Texture);

  unsigned int _texture_id;
  size_t _width;
  size_t _height;
  size_t _actual_width;
  size_t _actual_height;
  
  struct {
    unsigned repeated : 1;
    unsigned smooth : 1;
    unsigned flipped : 1;
  } _flags;
};

} // namespace bm

#endif//BLOWMORPH_CLIENT_TEXTURE_HPP_