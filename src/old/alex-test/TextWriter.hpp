#ifndef BLOWMORPH_TEXTWRITER_HPP_
#define BLOWMORPH_TEXTWRITER_HPP_

// OpenGL Header
#include <GL/glew.h>
#include <glm/glm.hpp>

#include <vector>
#include <string>
#include <cstdarg>

#include "base/error.hpp"

namespace freetype {
 
  using std::vector;
  using std::string;
   
  // This holds all of the information related to any
  // FreeType font that we want to create. 
  struct font_data {
    float h;                                        // Holds the height of the font.
    GLuint * textures;                              // Holds the texture id's.
    GLuint list_base;                               // Holds the first display list id.
   
    // The init function will create a font with the height h From The File 
    // fname.
    bool init(const char * fname, unsigned int h);
   
    // Free All The Resources Associated With The Font.
    void clean();
  };

  // This thing will print out text at window coordinates X, Y, using the font 
  // ft_font. The current modelview matrix will also be applied to the text.
  void print(const font_data &ft_font, float x, float y, glm::vec4 color, const char *fmt,  va_list arg_pointers);
  void print(const font_data &ft_font, float x, float y, glm::vec4 color, const char *fmt,  ...);
}  

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
  void PrintText(float x, float y, const char *text, ...);
  void PrintText(glm::vec4 color, float x, float y, const char *text, ...);
  
  // Set default printing string, used before Render(float, float) and Render().
  void SetPrintingString(std::string printing_string);

  // Draw default printing string from point (x, y). You need run 
  // SetPrintingString(string) one time before it.
  void Render(float x, float y);

  // Set default ccordinates of place, where default string will be drawn, 
  // used before Render().
  void SetCoordinates(float x, float y);

  // Draw default printing string from default point. You need run 
  // SetCoordinates(float, float) and SetPrintingString(string) one time before
  // it.
  void Render();

private:
  enum state {
    STATE_INITIALIZED,
    STATE_ERROR,
    STATE_DELETED
  };
  state _state;

  freetype::font_data _our_font;

  std::string _printing_string;
  float _x, _y;
  glm::vec4 _color;
};
#endif /*BLOWMORPH_TEXTWRITER_HPP_*/
