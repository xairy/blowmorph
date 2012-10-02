#ifndef BLOWMORPH_BASE_LEAK_DETECTOR_HPP_
#define BLOWMORPH_BASE_LEAK_DETECTOR_HPP_

#include <cstdlib>

#include "base.hpp"

namespace bm { namespace leak_detector {

BM_BASE_DECL void* Allocate(size_t size, bool array, const char* file, unsigned int line);
BM_BASE_DECL void Free(void* ptr, bool array, const char* file = 0, unsigned int line = 0);
BM_BASE_DECL void PrintAllLeaks();

} } // namespace bm::leak_detector

#ifndef LEAK_DETECTOR_PRIVATE
inline void* operator new(size_t size, const char* file, unsigned int line) {
  return bm::leak_detector::Allocate(size, false, file, line);
}
inline void* operator new[](size_t size, const char* file, unsigned int line) {
  return bm::leak_detector::Allocate(size, true, file, line);
}
inline void operator delete(void* ptr, const char* file, unsigned int line) {
  bm::leak_detector::Free(ptr, false, file, line);
}
inline void operator delete[](void* ptr, const char* file, unsigned int line) {
  bm::leak_detector::Free(ptr, true, file, line);
}
inline void operator delete(void* ptr) {
  bm::leak_detector::Free(ptr, false);
}
inline void operator delete[](void* ptr) {
  bm::leak_detector::Free(ptr, true);
}

#define new new(__FILE__, __LINE__)
#endif//LEAK_DETECTOR_PRIVATE

#endif//BLOWMORPH_BASE_LEAK_DETECTOR_HPP_
