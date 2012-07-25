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
  // You can use {Hawaii_Killer.ttf, tahoma.ttf, verdana.ttf} as file_name
  bool InitFont(std::string file_name, int pixel_size);

  // Run it before ending
  void Destroy();  
  
  // This method will print out text at window coordinates X, Y, using the font 
  // specified at initialization. The current modelview matrix will also be 
  // applied to the text.
  //void PrintText(float x, float y, const char *text, ...);
  void PrintText(glm::vec4 color, float x, float y, const char *text, ...);
  
  // Set default printing string, used before Render(float, float) and Render().
  //void SetPrintingString(std::string printing_string);

  // Draw default printing string from point (x, y). You need run 
  // SetPrintingString(string) one time before it.
  //void Render(float x, float y);

  // Set default ccordinates of place, where default string will be drawn, 
  // used before Render().
  //void SetCoordinates(float x, float y);

  // Draw default printing string from default point. You need run 
  // SetCoordinates(float, float) and SetPrintingString(string) one time before
  // it.
  //void Render();

private:
  enum state {
    STATE_INITIALIZED,
    STATE_ERROR,
    STATE_DELETED
  };
  state _state;
  
  struct FontData;
  FontData* _our_font;

  //std::string _printing_string;
  //float _x, _y;
  //glm::vec4 _color;
};

}; // namespace bm

#endif // BLOWMORPH_TEXTWRITER_HPP_
