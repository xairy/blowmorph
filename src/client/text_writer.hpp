#ifndef BLOWMORPH_TEXTWRITER_HPP_
#define BLOWMORPH_TEXTWRITER_HPP_

// OpenGL Header
#include <GL/glew.h>
#include <glm/glm.hpp>

#include <vector>
#include <string>
#include <cstdarg>

#include "base/error.hpp"

namespace bm {

class TextWriter {
public:
  TextWriter();
  ~TextWriter();

  // Run it after GL initialization
  bool InitFont(std::string file_name, int pixel_size);

  // Run it before ending
  void Destroy();
  
  // This method will print out text at window coordinates X, Y, using the font 
  // specified at initialization. The current modelview matrix will also be 
  // applied to the text.
  void PrintText(glm::vec4 color, float x, float y, const char *text, ...);

private:
  enum state {
    STATE_INITIALIZED,
    STATE_ERROR,
    STATE_DELETED
  };
  state _state;
  
  struct FontData;
  FontData* _our_font;
};

}; // namespace bm

#endif // BLOWMORPH_TEXTWRITER_HPP_
