#define LEAK_DETECTOR_PRIVATE
#include "leak_detector.hpp"

#include <cassert>
#include <vector>
#include <iostream>

namespace bm { namespace leak_detector {

struct AllocInfo {
  bool array;
  size_t size;
  const char* file;
  unsigned int line;    
  const void* ptr;
};
std::vector<AllocInfo> allocations;

void* Allocate(size_t size, bool array, const char* file, unsigned int line) {
  void* result;
  if (array) {
    result = ::operator new[](size);
  } else {
    result = ::operator new(size);
  }

  AllocInfo info;
  info.array = array;
  info.size = size;
  info.file = file;
  info.line = line;
  info.ptr = result;
  allocations.push_back(info);

  return result;
}
void Free(void* ptr, bool array, const char* file, unsigned int line) {
  typedef std::vector<AllocInfo>::iterator It;
  for (It i = allocations.begin(); i != allocations.end(); ++i) {
    if (i->ptr == ptr) {
      assert(i->array == array);
      allocations.erase(i);
      break;
    }
  }
  
  if (array) {
    ::operator delete[](ptr);
  } else {
    ::operator delete(ptr);
  }
}
void PrintAllLeaks() {
  typedef std::vector<AllocInfo>::const_iterator It;
  std::cout << "Leaks : " << std::endl;
  for (It i = allocations.begin(); i != allocations.end(); ++i) {
    std::cout << "Leak #" << i->ptr << std::endl <<
      "  array   : " << i->array << std::endl <<
      "  size    : " << i->size << std::endl <<
      "  file    : " << i->file << std::endl <<
      "  line    : " << i->line << std::endl; 
  }
}

} } // namespace bm::leak_detector
