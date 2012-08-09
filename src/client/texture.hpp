#ifndef BLOWMORPH_CLIENT_TEXTURE_HPP_
#define BLOWMORPH_CLIENT_TEXTURE_HPP_

#include <cstring>
#include <string>
#include <glm/glm.hpp>

#include <base/pstdint.hpp>
#include <base/macros.hpp>

namespace bm {

typedef glm::detail::tvec3<bm::uint8_t> rgb;
typedef glm::detail::tvec4<bm::uint8_t> rgba;
typedef glm::vec3 rgbf;
typedef glm::vec4 rgbaf;

SCHECK(sizeof(rgb) == 3);
SCHECK(sizeof(rgba) == 4);

// TODO[24.7.2012 alex]: move methods into a separate ".inl" file?
// XXX[24.7.2012 alex]: doesn't follow codestyle
template<class T>
class texture {
public:
  texture() {
    _data = NULL;
    _width = 0;
    _height = 0;
  }
  ~texture() {
    _free();
  }
  
  size_t width() const {
    return _width;
  }
  size_t height() const {
    return _height;
  }
  T* ptr() const {
    return _data;
  }
  size_t size() const {
    return sizeof(T) * _width * _height;
  }
  
  void setSize(size_t w, size_t h) {
    if (_width == w && _height == h) {
      return;
    }
    
    _free();
    _data = new T[w * h];
    _width = w;
    _height = h;
  }
  
  T& operator() (size_t x, size_t y) {
    CHECK(x < _width);
    CHECK(y < _height);
    
    return _data[y * _width + x];
  }
  const T& operator() (size_t x, size_t y) const {
    CHECK(x < _width);
    CHECK(y < _height);
    
    return _data[y * _width + x];
  }
  
private:
  T* _data;
  size_t _width;
  size_t _height;
  
  void _free() {
    if (_data != NULL) {
      delete[] _data;
      _data = NULL;
      _width = 0;
      _height = 0;
    }
  }
};

template<class T>
void copy(texture<T>& dst, const texture<T>& src) {
  // If it's the same texture, do nothing.
  if (dst.ptr() == src.ptr()) {
    return;
  }
  
  dst.setSize(src.width(), src.height());
  ::memcpy(dst.ptr(), src.ptr(), src.size());
}

// XXX[90.08.2012 xairy]: Is this code really needed?
/*
template<class U, class F>
void apply(texture<U>& tex, F f) {
  size_t width = tex.width();
  size_t height = tex.height();
  
  for (size_t y = 0; y < height; y++) {
    for (size_t x = 0; x < width; x++) {
      f(tex(x, y));
    }
  }
}

template<class U, class V, class F>
void unaryOp(texture<U>& dst, const texture<V>& src, F f) {
  CHECK(dst.width() == src.width());
  CHECK(dst.height() == src.height());
  
  size_t width = src.width();
  size_t height = src.height();
  
  for (size_t y = 0; y < height; y++) {
    for (size_t x = 0; x < width; x++) {
      dst(x, y) = f(src(x, y));
    }
  }
}
*/

bool LoadRGBA(texture<rgba>& dst, const std::string& path);
bool SaveRGBA(const std::string& path, const texture<rgba>& src);

}; // namespace bm

#endif // BLOWMORPH_CLIENT_TEXTURE_HPP_
