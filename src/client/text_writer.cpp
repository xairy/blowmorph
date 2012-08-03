#include "text_writer.hpp"

#include <map>

// FreeType Headers
#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftglyph.h>
#include <freetype/ftoutln.h>
#include <freetype/fttrigon.h>

#include <base/pstdint.hpp>

namespace {
  struct GlyphInfo {
    double advance_x;
    double advance_y;
    
    double offset_x;
    double offset_y;
    
    size_t size_x;
    size_t size_y;
    
    GLuint texture_id;
  };

  typedef std::map<bm::uchar, GlyphInfo> GlyphMap;

  // TODO[24.7.2012 alex]: separate into classes/structs for glyph info/font info
  // TODO[24.7.2012 alex]: rename everything to comply with codestyle
  
  // Holds all the information related to font.
  struct FontDataPrivate {
    float font_height;                              // Holds the height of the font.
    //GLuint * textures;                              // Holds the texture id's.
    //GLuint list_base;                               // Holds the first display list id.
   
    GlyphMap glyphs;
   
    // TODO[24.7.2012 alex]: move methods outside of the struct
    // The init function will create a font with the height h From The File 
    // fname.
    bool init(const char * fname, unsigned int h);
   
    // Free All The Resources Associated With The Font.
    void clean();
  };

  // XXX[24.7.2012 alex]: should we use bm::texture?
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

  void LoadGlyphInfo(FT_GlyphSlot glyph, GlyphInfo& info) {
    info.advance_x = glyph->advance.x / 64.0;
    info.advance_y = glyph->advance.y / 64.0;

    info.offset_x = glyph->bitmap_left;
    info.offset_y = glyph->bitmap_top;

    info.size_x = glyph->bitmap.width;
    info.size_y = glyph->bitmap.rows;
  }
  
  void LoadGlyphTexture(FT_GlyphSlot glyph, GlyphInfo& info) {
    // generate a texture
    glGenTextures(1, &info.texture_id);
    CHECK(info.texture_id != 0);
    
    // get texture data
    GLubyte* data = GetFTBitmapData(glyph->bitmap);
    
    // XXX[24.7.2012 alex]: should I use GL_TEXTURE_2D instead?
    // Now we just setup some texture parameters.
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, info.texture_id); 
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 

    // Here We Actually Create The Texture Itself, Notice That We Are Using 
    // GL_LUMINANCE_ALPHA To Indicate That We Are Using 2 Channel Data.
    glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGBA, info.size_x, info.size_y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data); 

    delete [] data; 
  }
  
  void LoadAsciiGlyphs(FT_Face face, FontDataPrivate& font_data) {
    for (bm::uchar ch = 1; ch <= 127; ch++) {
      // Load the glyph for our character.
      if (FT_Load_Glyph(face, FT_Get_Char_Index(face, ch), FT_LOAD_RENDER))
        BM_ERROR("FT_Load_Glyph failed");

      GlyphInfo info;
      LoadGlyphInfo(face->glyph, info);
      LoadGlyphTexture(face->glyph, info);

      font_data.glyphs[ch] = info;
    }
  }
  
  void UnloadGlyphs(GlyphMap& glyphs) {
    for (GlyphMap::iterator it = glyphs.begin(); it != glyphs.end(); ++it) {
      glDeleteTextures(1, &it->second.texture_id);
    }
    glyphs.clear();
  }
  
  void RenderGlyph(const GlyphInfo& info) {
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, info.texture_id); 
    glPushMatrix(); 

    // First we need to move over a little so that the character has the right
    // amount of space between it and the one before it.
    glTranslated(info.offset_x, 0, 0);

    // Now we move down a little in the case that the bitmap extends past the 
    // bottom of the line this is only true for characters like 'g' or 'y'.
    glTranslated(0, -info.offset_y, 0);

    // Here we draw the texturemapped quads. The bitmap that we got from 
    // FreeType was not oriented quite like we would like it to be, but we link
    // the texture to the quad in such a way that the result will be properly 
    // aligned.
    glBegin(GL_QUADS);
      glTexCoord2d(0, 0);  
      glVertex2d(0, info.size_y);

      glTexCoord2d(0, info.size_y);
      glVertex2d(0, 0);

      glTexCoord2d(info.size_x, info.size_y);  
      glVertex2d(info.size_x, 0); 

      glTexCoord2d(info.size_x, 0);  
      glVertex2d(info.size_x, info.size_y);
    glEnd();

    glPopMatrix(); 

    glTranslated(info.advance_x, 0, 0); 
  }

  /*// TODO[24.7.2012 alex]: would be great to decompose this long method

  // The makeDisplayCharacterList takes in an FT_Face, which is an object that 
  // FreeType uses to store information about a font, and creates a display 
  // list coresponding to the character which we pass in.
  void makeDisplayCharacterList(FT_Face face, char ch, GLuint list_base, GLuint * tex_base){
    // TODO[24.7.2012 alex]: fix comments
    // The First Thing We Do Is Get FreeType To Render Our Character Into 
    // Bitmap. This Actually Requires A Couple Of FreeType Commands:
    
    // Load The Glyph For Our Character.
    if(FT_Load_Glyph (face, FT_Get_Char_Index (face, ch), FT_LOAD_DEFAULT))
        BM_ERROR("FT_Load_Glyph failed");

    // Move The Face's Glyph Into A Glyph Object.
    FT_Glyph glyph; 
    if(FT_Get_Glyph (face-> glyph, &glyph))
        BM_ERROR("FT_Get_Glyph failed"); 

    // Convert The Glyph To A Bitmap.
    FT_Glyph_To_Bitmap (&glyph, ft_render_mode_normal, 0, 1); 
    FT_BitmapGlyph bitmap_glyph = (FT_BitmapGlyph)glyph;
    
    FT_Bitmap& bitmap = bitmap_glyph->bitmap;
    int width = bitmap.width;
    int height = bitmap.rows;
    
    GLubyte* data = GetFTBitmapData(bitmap);
    
    // XXX[24.7.2012 alex]: should I use GL_TEXTURE_2D instead?
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
  
    // XXX[24.7.2012 alex]: not GL3 complaint i'm afraid
    // Now We Create The Display List
    glNewList(list_base + ch, GL_COMPILE); 
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, tex_base[ch]); 
    glPushMatrix(); 

    // First We Need To Move Over A Little So That The Character Has The Right
    // Amount Of Space Between It And The One Before It.
    glTranslatef((GLfloat) bitmap_glyph->left, 0, 0);

    // Now We Move Down A Little In The Case That The Bitmap Extends Past The 
    // Bottom Of The Line This Is Only True For Characters Like 'g' Or 'y'.
    glTranslatef(0, (GLfloat) -bitmap_glyph->top, 0);

    // Here We Draw The Texturemapped Quads. The Bitmap That We Got From 
    // FreeType Was Not Oriented Quite Like We Would Like It To Be, But We Link
    // The Texture To The Quad In Such A Way That The Result Will Be Properly 
    // Aligned.
    glBegin(GL_QUADS);
      // XXX[24.7.2012 alex]: don't like these checks here
      // just to be sure, let's check it
      SCHECK(sizeof(bitmap.width) == sizeof(GLint));
      SCHECK(sizeof(bitmap.rows) == sizeof(GLint));
    
      glTexCoord2i(0, 0);  
      glVertex2f(0, (GLfloat) bitmap.rows);
      
      glTexCoord2i(0, (GLint) bitmap.rows);
      glVertex2f(0, 0);

      glTexCoord2i((GLint) bitmap.width, (GLint) bitmap.rows);  
      glVertex2f((GLfloat) bitmap.width, 0); 

      glTexCoord2i((GLint) bitmap.width, 0);  
      glVertex2f((GLfloat) bitmap.width, (GLfloat) bitmap.rows);
    glEnd();
    
    glPopMatrix(); 
    
    glTranslatef(face->glyph->advance.x / 64.0f, 0, 0); 

    // Increment The Raster Position As If We Were A Bitmap Font. (Only Needed 
    // If You Want To Calculate Text Length).
    // glBitmap(0, 0, 0, 0, face-> glyph-> advance.x  >>  6, 0, NULL); 

    // Finish The Display List
    glEndList(); 
  }*/

  bool FontDataPrivate::init(const char * fname, unsigned int height) {
    // Allocate Some Memory To Store The Texture Ids.
    // XXX[24.7.2012 alex]: unicode?
    //textures = new GLuint[128]; 
 
    this-> font_height = (float) height;
 
    // XXX[24.7.2012 alex]: init library for every font?
    // Create And Initilize A FreeType Font Library.
    FT_Library library; 
    if (FT_Init_FreeType (&library)) {
       BM_ERROR("FT_Init_FreeType failed");
       return false;
    }
 
    // The Object In Which FreeType Holds Information On A Given Font Is Called
    // A "face".
    FT_Face face; 
 
    // This Is Where We Load In The Font Information From The File. Of All The 
    // Places Where The Code Might Die, This Is The Most Likely, As FT_New_Face
    // Will Fail If The Font File Does Not Exist Or Is Somehow Broken.
    if (FT_New_Face(library, fname, 0, &face)) {
       BM_ERROR("FT_New_Face failed (there is probably a problem with your font file)"); 
       return false;
    }
    
    // For Some Twisted Reason, FreeType Measures Font Size In Terms Of 1/64ths
    // Of Pixels.  Thus, To Make A Font height Pixels High, We Need To Request
    // A Size Of font_height * 64.
    FT_Set_Char_Size(face, height << 6, height << 6, 90, 90); 
 
    LoadAsciiGlyphs(face, *this);
 
    //list_base = glGenLists(128); 
    //glGenTextures (128, textures); 
 
    // This Is Where We Actually Create Each Of The Fonts Display Lists.
    //for(unsigned char i = 0; i < 128; i++)
        //makeDisplayCharacterList(face, i, list_base, textures); 
 
    // We Don't Need The Face Information Now That The Display Lists Have Been
    // Created, So We Free The Assosiated Resources.
    FT_Done_Face(face); 
 
    // Ditto For The Font Library.
    FT_Done_FreeType(library); 
    return true;
  }
  
  void FontDataPrivate::clean() {
    //glDeleteLists(list_base, 128); 
    //glDeleteTextures(128, textures); 
    //delete [] textures;
    UnloadGlyphs(glyphs);
  }

  // Coordinates Identical To Window Coordinates.
  void pushScreenCoordinateMatrix() {
      glPushAttrib(GL_TRANSFORM_BIT); 
      GLint   viewport[4]; 
      glGetIntegerv(GL_VIEWPORT, viewport); 
      glMatrixMode(GL_PROJECTION); 
      glPushMatrix();
      glLoadIdentity();
      // XXX[24.7.2012 alex]: looks completely unreadable
      glOrtho(viewport[0], viewport[0] + viewport[2],
              viewport[1] + viewport[3], viewport[1],
              -1, 1);
      glPopAttrib(); 
  }
   
  // Pops current projection matrix without changing the current matrix mode.
  void popProjectionMatrix() {
      glPushAttrib(GL_TRANSFORM_BIT); 
      glMatrixMode(GL_PROJECTION); 
      glPopMatrix(); 
      glPopAttrib(); 
  }
  
  std::vector<std::string> SplitLines(const char* text) {
    // Split text into lines.   
    const char *start_line = text; 
    std::vector<std::string> lines; 
    const char* c;
    
    for (c = text; *c; c++) {
      if (*c == '\n') {
        std::string line;
        for(const char *n = start_line; n < c; n++) line.append(1, *n); 
        lines.push_back(line); 
        start_line = c + 1; 
      }
    }
    if (start_line) {
      std::string line; 
      for(const char *n = start_line; n < c; n++) line.append(1, *n); 
      lines.push_back(line); 
    }
    
    return lines;
  }
  
  void RenderText(const FontDataPrivate &font, float x, float y, const glm::vec4& color, const char* text)  {
      CHECK(text != NULL);
      
      // We Want A Coordinate System Where Distance Is Measured In Window Pixels.
      pushScreenCoordinateMatrix();                                   
      
      //GLuint list_base = font.list_base;
      // We Make The Height A Little Bigger. There Will Be Some Space Between Lines.
      float line_height = font.font_height / .63f;
      
      std::vector<std::string> lines = SplitLines(text);
   
      glPushAttrib(GL_LIST_BIT | GL_CURRENT_BIT  | GL_ENABLE_BIT | GL_TRANSFORM_BIT); 
      glMatrixMode(GL_MODELVIEW); 
      glDisable(GL_LIGHTING); 
      glEnable(GL_TEXTURE_2D); 
      glDisable(GL_DEPTH_TEST); 
      glEnable(GL_BLEND); 
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);      
   
      //glListBase(list_base); 

      float modelview_matrix[16];     
      glGetFloatv(GL_MODELVIEW_MATRIX, modelview_matrix); 
 
      // This Is Where The Text Display Actually Happens. For Each Line Of Text
      // We Reset The Modelview Matrix So That The Line's Text Will Start In 
      // The Correct Position. Notice That We Need To Reset The Matrix, Rather 
      // Than Just Translating Down By font_height. This Is Because When Each Character
      // Is Drawn It Modifies The Current Matrix So That The Next Character
      // Will Be Drawn Immediately After It. 
      
      glColor4f(color.r, color.g, color.b, color.a);
      for(size_t i = 0; i < lines.size(); i++) {
        glPushMatrix();
        glLoadIdentity();
        glTranslatef(x, y + line_height * i, 0); 
        glMultMatrixf(modelview_matrix);
        // XXX[29.7.2012 alex]: casts size_t to GLsizei
        for (size_t k = 0; k < lines[i].size(); k++) {
          bm::uchar ch = lines[i][k];
          GlyphMap::const_iterator it = font.glyphs.find(ch);
          if (it != font.glyphs.end()) {
            RenderGlyph(it->second);
          }
        }
        //glCallLists(static_cast<GLsizei>(lines[i].length()), GL_UNSIGNED_BYTE, lines[i].c_str()); 
        glPopMatrix();
      }
   
      glPopAttrib();
   
      popProjectionMatrix();
  }

}; // anonymous namespace

namespace bm {

struct TextWriter::FontData {
  FontDataPrivate fontData;
};

TextWriter::TextWriter() { }

TextWriter::~TextWriter() {
  CHECK(_state = STATE_DELETED);
}

bool TextWriter::InitFont(std::string file_name, int pixel_size) {
  _our_font = new FontData();
  
  if(!_our_font->fontData.init(file_name.c_str(), pixel_size)) {
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
    _our_font->fontData.clean();
    delete _our_font;
  }
  _state = STATE_DELETED;
}

void TextWriter::PrintText(glm::vec4 color, float x, float y, const char *fmt, ...) {
  CHECK(_state == STATE_INITIALIZED);

  char buf[1024];
  va_list args;
  va_start(args, fmt);  
  vsnprintf(buf, sizeof(buf) - 1, fmt, args);
  va_end(args);
  
  RenderText(_our_font->fontData, x, y, color, buf);
}

}; // namespace bm