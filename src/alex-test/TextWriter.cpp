#include "TextWriter.hpp"

// FreeType Headers
#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftglyph.h>
#include <freetype/ftoutln.h>
#include <freetype/fttrigon.h>

void Error(const char* error) {
  fprintf(stderr, "%s\n", error);
}

namespace freetype {
  GLubyte* GetFTBitmapData(FT_Bitmap& bitmap) {
    // We are including an alpha channel so that the black parts of the bitmap 
    // will be transparent, and so that the edges of the text will be 
    // slightly translucent (which should make them look right against any 
    // background).
    int width = bitmap.width;
    int height = bitmap.rows;
    
    GLubyte* data = new GLubyte[4 * width * height]; 

    for(int y = 0; y < height; y++) {
      for(int x = 0; x < width; x++) {
        GLubyte value = bitmap.buffer[x + bitmap.pitch * (bitmap.rows - 1 - y)];
        
        data[4 * (x + y * width) + 0] = value;
        data[4 * (x + y * width) + 1] = value;
        data[4 * (x + y * width) + 2] = value;
        data[4 * (x + y * width) + 3] = value;
      }
    }
    
    return data;
  }

  // The makeDisplayCharacterList takes in an FT_Face, which is an object that 
  // FreeType uses to store information about a font, and creates a display 
  // list coresponding to the character which we pass in.
 
  // Create A Display List Corresponding To The Given Character.
  void makeDisplayCharacterList(FT_Face face, char ch, GLuint list_base, GLuint * tex_base){
   
    // The First Thing We Do Is Get FreeType To Render Our Character Into 
    // Bitmap. This Actually Requires A Couple Of FreeType Commands:

    // Load The Glyph For Our Character.
    if(FT_Load_Glyph (face, FT_Get_Char_Index (face, ch), FT_LOAD_DEFAULT))
        Error("FT_Load_Glyph failed"); 

    // Move The Face's Glyph Into A Glyph Object.
    FT_Glyph glyph; 
    if(FT_Get_Glyph (face-> glyph, &glyph))
        Error("FT_Get_Glyph failed"); 

    // Convert The Glyph To A Bitmap.
    FT_Glyph_To_Bitmap (&glyph, ft_render_mode_normal, 0, 1); 
    FT_BitmapGlyph bitmap_glyph = (FT_BitmapGlyph)glyph;
    
    FT_Bitmap& bitmap = bitmap_glyph->bitmap;
    int width = bitmap.width;
    int height = bitmap.rows;
    
    GLubyte* data = GetFTBitmapData(bitmap);

    // Now We Just Setup Some Texture Parameters.
    glBindTexture (GL_TEXTURE_RECTANGLE_ARB, tex_base[ch]); 
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 
     
    // Here We Actually Create The Texture Itself, Notice That We Are Using 
    // GL_LUMINANCE_ALPHA To Indicate That We Are Using 2 Channel Data.
    glTexImage2D (GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data); 
    
    delete [] data; 
    
    // We use texture mapped quads to draw our text. This means that it will be
    // easy to rotate and scale text, and it will also make fonts inherit their
    // color from the current OpenGL color (none of which would be true if we 
    // used pixmaps).
  
    // Now We Create The Display List
    glNewList(list_base + ch, GL_COMPILE); 
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, tex_base[ch]); 
    glPushMatrix(); 

    // First We Need To Move Over A Little So That The Character Has The Right
    // Amount Of Space Between It And The One Before It.
    glTranslatef(bitmap_glyph->left, 0, 0);

    // Now We Move Down A Little In The Case That The Bitmap Extends Past The 
    // Bottom Of The Line This Is Only True For Characters Like 'g' Or 'y'.
    glTranslatef(0, -bitmap_glyph->top, 0);

    // Here We Draw The Texturemapped Quads. The Bitmap That We Got From 
    // FreeType Was Not Oriented Quite Like We Would Like It To Be, But We Link
    // The Texture To The Quad In Such A Way That The Result Will Be Properly 
    // Aligned.
    glBegin(GL_QUADS);
      glTexCoord2i(0, 0);  
      glVertex2f(0, bitmap.rows);
      
      glTexCoord2i(0, bitmap.rows);
      glVertex2f(0, 0);

      glTexCoord2i(bitmap.width, bitmap.rows);  
      glVertex2f(bitmap.width, 0); 

      glTexCoord2i(bitmap.width, 0);  
      glVertex2f(bitmap.width, bitmap.rows);
    glEnd();
    
    glPopMatrix(); 
    
    glTranslatef(face->glyph->advance.x / 64, 0, 0); 

    // Increment The Raster Position As If We Were A Bitmap Font. (Only Needed 
    // If You Want To Calculate Text Length).
    // glBitmap(0, 0, 0, 0, face-> glyph-> advance.x  >>  6, 0, NULL); 

    // Finish The Display List
    glEndList(); 
  }

  bool font_data::init(const char * fname, unsigned int height) {
    // Allocate Some Memory To Store The Texture Ids.
    textures = new GLuint[128]; 
 
    this-> h = height; 
 
    // Create And Initilize A FreeType Font Library.
    FT_Library library; 
    if (FT_Init_FreeType (&library)) {
       Error("FT_Init_FreeType failed");
       return false;
    }
 
    // The Object In Which FreeType Holds Information On A Given Font Is Called
    // A "face".
    FT_Face face; 
 
    // This Is Where We Load In The Font Information From The File. Of All The 
    // Places Where The Code Might Die, This Is The Most Likely, As FT_New_Face
    // Will Fail If The Font File Does Not Exist Or Is Somehow Broken.
    if (FT_New_Face(library, fname, 0, &face)) {
       Error("FT_New_Face failed (there is probably a problem with your font file)"); 
       return false;
    }
    // For Some Twisted Reason, FreeType Measures Font Size In Terms Of 1/64ths
    // Of Pixels.  Thus, To Make A Font height Pixels High, We Need To Request
    // A Size Of h * 64.
    FT_Set_Char_Size(face, height << 6, height << 6, 90, 90); 
 
    list_base = glGenLists(128); 
    glGenTextures (128, textures); 
 
    // This Is Where We Actually Create Each Of The Fonts Display Lists.
    for(unsigned char i = 0; i < 128; i++)
        makeDisplayCharacterList(face, i, list_base, textures); 
 
    // We Don't Need The Face Information Now That The Display Lists Have Been
    // Created, So We Free The Assosiated Resources.
    FT_Done_Face(face); 
 
    // Ditto For The Font Library.
    FT_Done_FreeType(library); 
    return true;
  }
  
  void font_data::clean() {
    glDeleteLists(list_base, 128); 
    glDeleteTextures(128, textures); 
    delete [] textures; 
  }

  // Coordinates Identical To Window Coordinates.
  inline void pushScreenCoordinateMatrix() {
      glPushAttrib(GL_TRANSFORM_BIT); 
      GLint   viewport[4]; 
      glGetIntegerv(GL_VIEWPORT, viewport); 
      glMatrixMode(GL_PROJECTION); 
      glPushMatrix();
      glLoadIdentity();
      glOrtho(viewport[0], viewport[0] + viewport[2],
              viewport[1] + viewport[3], viewport[1],
              -1, 1);
      glPopAttrib(); 
  }
   
  // Pops The Projection Matrix Without Changing The Current MatrixMode.
  inline void pop_projection_matrix() {
      glPushAttrib(GL_TRANSFORM_BIT); 
      glMatrixMode(GL_PROJECTION); 
      glPopMatrix(); 
      glPopAttrib(); 
  }
  
  void print(const font_data &ft_font, float x, float y, glm::vec4 color, const char *fmt, va_list arg_pointers)  {
           
      // We Want A Coordinate System Where Distance Is Measured In Window Pixels.
      pushScreenCoordinateMatrix();                                   
           
      GLuint font = ft_font.list_base; 
      // We Make The Height A Little Bigger. There Will Be Some Space Between Lines.
      float h = ft_font.h/.63f;                                                 
      char text[256];                                   
                                     
      // If There's No Text Do Nothing
      if (fmt  ==  NULL)
          *text = 0; 
      else {
      // Parses The String For Variables And Converts Symbols To Actual Numbers
      // Results Are Stored In Text.
          vsprintf(text, fmt, arg_pointers);                             
      }

      // Split text into lines.   
      const char *start_line = text; 
      vector < string >  lines; 
      char* c;
      for(c = text; *c; c++) {
        if(*c == '\n') {
          string line; 
          for(const char *n = start_line; n < c; n++) line.append(1, *n); 
          lines.push_back(line); 
          start_line = c + 1; 
        }
      }
      if(start_line) {
        string line; 
        for(const char *n = start_line; n < c; n++) line.append(1, *n); 
        lines.push_back(line); 
      }
   
      glPushAttrib(GL_LIST_BIT | GL_CURRENT_BIT  | GL_ENABLE_BIT | GL_TRANSFORM_BIT); 
      glMatrixMode(GL_MODELVIEW); 
      glDisable(GL_LIGHTING); 
      glEnable(GL_TEXTURE_2D); 
      glDisable(GL_DEPTH_TEST); 
      glEnable(GL_BLEND); 
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);      
   
      glListBase(font); 

      float modelview_matrix[16];     
      glGetFloatv(GL_MODELVIEW_MATRIX, modelview_matrix); 
 
      // This Is Where The Text Display Actually Happens. For Each Line Of Text
      // We Reset The Modelview Matrix So That The Line's Text Will Start In 
      // The Correct Position. Notice That We Need To Reset The Matrix, Rather 
      // Than Just Translating Down By h. This Is Because When Each Character
      // Is Drawn It Modifies The Current Matrix So That The Next Character
      // Will Be Drawn Immediately After It. 
      
      glColor4f(color.r, color.g, color.b, color.a);
      for(int i = 0; i < lines.size(); i++) {
        glPushMatrix(); 
        glLoadIdentity(); 
        glTranslatef(x, y + h * i, 0); 
        glMultMatrixf(modelview_matrix); 
   
        // The Commented Out Raster Position Stuff Can Be Useful If You Need To
        // Know The Length Of The Text That You Are Creating.
        // If You Decide To Use It Make Sure To Also Uncomment The glBitmap Command
        // In makeDisplayCharacterList().

        //glRasterPos2f(0, 0); 
        glCallLists(lines[i].length(), GL_UNSIGNED_BYTE, lines[i].c_str()); 
        //float rpos[4]; 
        //glGetFloatv(GL_CURRENT_RASTER_POSITION , rpos); 
        glPopMatrix(); 
      }
   
      glPopAttrib();          
   
      pop_projection_matrix(); 
  }
  
  void print(const font_data &ft_font, float x, float y, glm::vec4 color, const char *fmt,  ...) {
    va_list arg_pointers;
    va_start(arg_pointers, fmt);
    print(ft_font, x, y, glm::vec4(1, 1, 1, 1), fmt, arg_pointers);
    va_end(arg_pointers);
  }

}  // Close the namespace freetype

TextWriter::TextWriter() {
 _printing_string = "";
 _x = _y = -1;
 _color = glm::vec4(1, 1, 1, 1);
};

TextWriter::~TextWriter() {
  CHECK(_state = STATE_DELETED);
}

bool TextWriter::InitFont(std::string file_name, int pixel_size) {
  if(!_our_font.init(file_name.c_str(), pixel_size)) {
    fprintf(stderr, "Coudn't intialize font\n");
    _state = STATE_ERROR;
    return false;
  }
  else {
    _state = STATE_INITIALIZED;
    return true;
  }
}

void TextWriter::Destroy() {
  if (_state == STATE_INITIALIZED) {
    _our_font.clean();
  }
  _state = STATE_DELETED;
}

void TextWriter::PrintText(float x, float y, const char *text, ...) {
  CHECK(_state == STATE_INITIALIZED);
  
  va_list arg_pointers;
  va_start(arg_pointers,  text);  
  freetype::print(_our_font, x, y, _color, text, arg_pointers);
  va_end(arg_pointers);
} 

void TextWriter::PrintText(glm::vec4 color, float x, float y, const char *text, ...) {
  CHECK(_state == STATE_INITIALIZED);
  
  va_list arg_pointers;
  va_start(arg_pointers,  text);  
  freetype::print(_our_font, x, y, color, text, arg_pointers);
  va_end(arg_pointers);
}

void TextWriter::SetPrintingString(std::string printing_string) {
  _printing_string = printing_string;
}

void TextWriter::Render(float x, float y) {
  freetype::print(_our_font, x, y, _color, "%s", _printing_string.c_str());
}

void TextWriter::SetCoordinates(float x, float y) {
  _x = x;
  _y = y;
}

void TextWriter::Render() {
  if(_x < 0 || _y < 0) {
    fprintf(stderr, "Uninitialized or non-correct coordinates\n");
    return;
  }
  freetype::print(_our_font, _x, _y, _color, "%s", _printing_string.c_str());
}