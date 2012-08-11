#ifndef BLOWMORPH_CLIENT_ARRAY_HPP_
#define BLOWMORPH_CLIENT_ARRAY_HPP_

#include <base/macros.hpp>

namespace bm {

// TODO[24.7.2012 alex, 11.08.2012 xairy]: move methods into
// a separate ".inl" file?
template<class T>
class Array2 {
public:
  Array2() {
    _data = NULL;
    _width = 0;
    _height = 0;
  }
  ~Array2() {
    free();
  }
  
  size_t Width() const {
    return _width;
  }
  size_t Height() const {
    return _height;
  }
  T* Data() const {
    return _data;
  }
  size_t Size() const {
    return sizeof(T) * _width * _height;
  }
  
  void SetSize(size_t w, size_t h) {
    if (_width == w && _height == h) {
      return;
    }
    
    free();
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
  
  void free() {
    if (_data != NULL) {
      delete[] _data;
      _data = NULL;
      _width = 0;
      _height = 0;
    }
  }
};

} // namespace bm

#endif // BLOWMORPH_CLIENT_ARRAY_HPP_
