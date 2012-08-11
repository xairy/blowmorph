#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstdlib>

#include <algorithm>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>

#include <GL/glew.h>
#include <SDL/SDL.h>
#include <glm/glm.hpp>
#include <FreeImage.h>
#include <time.h>

#define _USE_MATH_DEFINES
#include <math.h>

#include "TextureManager.hpp"
#include "Sprite.hpp"
#include "glmext.hpp"
#include "TextWriter.hpp"


bm::TextureManager* manager;
bm::Texture* image;
bm::Texture* tiles;
bm::Sprite* sprite;
std::vector<bm::Sprite*> tileSprites;

glm::float_t lastUpdate;
glm::vec2 position;
glm::float_t speed;
glm::float_t angle;

TextWriter textWriter;

void raise_error(const char* error) {
  fprintf(stderr, "%s\n", error);
}

void Reshape(int w, int h) {
  glViewport(0, 0, (GLsizei) w, (GLsizei) h);
}

void Enable2D() {
  GLint viewport[4];
  glGetIntegerv(GL_VIEWPORT, viewport);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(viewport[0], viewport[0] + viewport[2],
          viewport[1] + viewport[3], viewport[1],
          -1, 1);
          
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_TEXTURE_RECTANGLE_ARB);
  
  textWriter.InitFont("data/tahoma.ttf", 20);
}

class Application {
public:
  Application() : _is_running(false) { }
  ~Application() { }

  bool Execute() {
    if(!Initialize()) {
      return false;
    }

    _is_running = true;

    while(_is_running) {
      PumpEvents();
      Loop();
      Render();
    }

    Destroy();

    return true;
  }

private:
  bool InitializeSDL() {
    if(SDL_Init(SDL_INIT_VIDEO) < 0) {
      raise_error("Unable to initialize SDL!");
      return false;
    }
    
    const SDL_VideoInfo *videoInfo = SDL_GetVideoInfo();
    if (!videoInfo) {
      raise_error("Unable to get video info!");
      return false;
    }
    
    int videoFlags;
    /* the flags to pass to SDL_SetVideoMode */
    videoFlags  = SDL_OPENGL;          /* Enable OpenGL in SDL */
    videoFlags |= SDL_GL_DOUBLEBUFFER; /* Enable double buffering */
    videoFlags |= SDL_HWPALETTE;       /* Store the palette in hardware */
    
    /* This checks to see if surfaces can be stored in memory */
    if ( videoInfo->hw_available )
      videoFlags |= SDL_HWSURFACE;
    else
      videoFlags |= SDL_SWSURFACE;
    
    /* This checks if hardware blits can be done */
    if ( videoInfo->blit_hw )
	    videoFlags |= SDL_HWACCEL;
    
    /* Sets up OpenGL double buffering */
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1 );
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    
    sdlSurface = SDL_SetVideoMode(512, 512, 24, videoFlags);
    if (!sdlSurface) {
      raise_error("Unable to set video mode!");
      return false;
    }

    // Create our window centered at 512x512 resolution
    /*_sdl_window = SDL_CreateWindow("Blowmorph Demo", SDL_WINDOWPOS_CENTERED,
      SDL_WINDOWPOS_CENTERED, 512, 512, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if(_sdl_window == NULL) {
      raise_error("Unable to create window!");
      return false;
    }

    // Create our opengl context and attach it to our window.
    _sdl_context = SDL_GL_CreateContext(_sdl_window);
    if(_sdl_context == NULL) {
      raise_error("Unable to create context!");
      return false;
    }

    // This makes our buffer swap syncronized with the monitor's vertical refresh.
    SDL_GL_SetSwapInterval(1);*/

    // Set up glew.
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if(GLEW_OK != err) {
      raise_error("Unable to init GLEW!");
      return false;
    }
    
    // XXX
    Reshape(512, 512);
    Enable2D();
    
    return true;
  }
  bool InitializeFreeImage() {
    FreeImage_Initialise();
    return true;
  }

  bool Initialize() {
    if (!InitializeSDL()) {
      return false;
    }
    
    if (!InitializeFreeImage()) {
      return false;
    }

    manager = new bm::TextureManager();
    image = manager->Load("data/plane.png", 0, 0, 0, 0, 0, 0, (8 << 16) + (54 << 8) + 129);
    tiles = manager->Load("data/blocks1.png", 2, 2, 34, 34, 32, 32, 0);
    
    for (int i = 0; i < 10; i++) {
      for (int k = 0; k < 10; k++) {
        bm::Sprite* newSprite = new bm::Sprite();
        
        if (!newSprite->Init(tiles, i * 10 + k)) {
          raise_error("Could not create sprite");
        }
        
        newSprite->SetPosition(glm::vec2(32 * i, 32 * k));
        tileSprites.push_back(newSprite);
      }
    }
    
    if (image == NULL) {
      raise_error("Could not load texture");
    }
    
    sprite = new bm::Sprite();
    if (!sprite->Init(image, 0)) {
      raise_error("Could not create sprite");
    }
    
    sprite->SetPivot(glm::vec2(0.5f, 0.5f));
    position = glm::vec2(100, 100);
    speed = 10;
    angle = 0;
    
    lastUpdate = SDL_GetTicks() / 100.0f;
    
    return true;
  }

  void ProcessEvent(SDL_Event* event) {
    switch(event->type) {
      case SDL_KEYDOWN:
        switch(event->key.keysym.sym){
          case SDLK_LEFT:
            angle -= glm::float_t(0.1);
            break;
          case SDLK_RIGHT:
            angle += glm::float_t(0.1);
            break;
          case SDLK_UP:
            speed += 1;
            break;
          case SDLK_DOWN:
            speed -= 1;
            break;
          case SDLK_ESCAPE:
            _is_running = false;
            break;
        }
        break;
      case SDL_QUIT:
        _is_running = false;
        break;
      /*case SDL_WINDOWEVENT:
        switch (event->window.event) {
          case SDL_WINDOWEVENT_RESIZED:
            Reshape(event->window.data1, event->window.data2);
            break;
        }
        break;*/
    }
  }

  void PumpEvents() {
    SDL_Event event;
    while(SDL_PollEvent(&event)) {
      ProcessEvent(&event);
    }
  }

  void Loop() {
    glm::float_t time = SDL_GetTicks() / 100.0f;
    glm::float_t dt = time - lastUpdate;
    lastUpdate = time;
    
    position += glm::vec2(speed * glm::cos(angle),
                          speed * glm::sin(angle)) * dt;
    
    sprite->SetPosition(position);
    sprite->SetAngle(angle + M_PI / 2);
  }

  void Render() {
    glClear(GL_COLOR_BUFFER_BIT);
  
    for (size_t i = 0; i < tileSprites.size(); i++) {
      tileSprites[i]->Render();
    }
  
    sprite->Render();
    
    int i = 4;
    
    textWriter.PrintText(glm::vec4(1, 0, 1, 0.5), 0, 155, "Coca-Cola, it's your bro! %d", i);
    
    std::string a = "Pepsi, it's not your bro!\nRDkL, alex, xairy, Kroll";
    textWriter.PrintText(glm::vec4(0, 1, 1, 0.8), 0, 105, "%s", a.c_str());

    textWriter.SetPrintingString("Ololo");
    textWriter.Render(0, 400);

    textWriter.SetPrintingString("Ololo!!!11!1");
    textWriter.SetCoordinates (0, 450);
    textWriter.Render();

  

    SwapBuffers();
  }

  void Destroy() {
    FreeImage_DeInitialise();
    textWriter.Destroy();  
    //SDL_GL_DeleteContext(_sdl_context);
    //SDL_DestroyWindow(_sdl_window);
    SDL_Quit();
  }

  void SwapBuffers() {
    SDL_GL_SwapBuffers();
  }

private:
  SDL_Surface* sdlSurface;

  bool _is_running;
};

int main(int argc, char** argv) {
  Application app;
  if(!app.Execute()) {
    //_CrtDumpMemoryLeaks();
    return EXIT_FAILURE;
  }
  //_CrtDumpMemoryLeaks();
  return EXIT_SUCCESS;
}