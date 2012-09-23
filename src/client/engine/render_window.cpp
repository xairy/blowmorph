#include "render_window.hpp"

#include <base/error.hpp>
#include <base/macros.hpp>

namespace bm {

RenderWindow::RenderWindow() {
  _sdl_surface = NULL;
  _state = STATE_FINALIZED;
}

RenderWindow::~RenderWindow() {
  if(_state == STATE_INITIALIZED) {
    Finalize();
  }
}

bool RenderWindow::Initialize(const char* title, int width, int height, bool fullscreen) {
  CHECK(_state == STATE_FINALIZED);
  CHECK(title != NULL);
  CHECK(width > 0 && height > 0);

  if (!initSDL(title, width, height, fullscreen)) {
    // XXX : error chain?
    return false;
  }

  if (!initGLEW()) {
    return false;
  }

  if (!initOpenGL(width, height)) {
    return false;
  }

  _state = STATE_INITIALIZED;
  return true;
}

void RenderWindow::Finalize() {
  CHECK(_state == STATE_INITIALIZED);

  SDL_Quit();
}

void RenderWindow::SwapBuffers() {
  CHECK(_state == STATE_INITIALIZED);

  SDL_GL_SwapBuffers();
}

bool RenderWindow::initSDL(const char* title, int width, int height, bool fullscreen) {
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    BM_ERROR("Unable to initialize SDL!");
    return false;
  }

  SDL_WM_SetCaption(title, NULL);

  const SDL_VideoInfo* videoInfo = SDL_GetVideoInfo();
  if (videoInfo == NULL) {
    BM_ERROR("Unable to get video info.");
    return false;
  }

  int videoFlags;
  // The flags to pass to 'SDL_SetVideoMode'.
  videoFlags  = SDL_OPENGL;          // Enable OpenGL in SDL.
  videoFlags |= SDL_GL_DOUBLEBUFFER; // Enable double buffering.
  videoFlags |= SDL_HWPALETTE;       // Store the palette in hardware.

  // This checks to see if surfaces can be stored in memory.
  if ( videoInfo->hw_available )
    videoFlags |= SDL_HWSURFACE;
  else
    videoFlags |= SDL_SWSURFACE;

  // This checks if hardware blits can be done.
  if ( videoInfo->blit_hw )
    videoFlags |= SDL_HWACCEL;

  // Sets up OpenGL double buffering.
  if (SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1 ) != 0) {
    BM_ERROR("Unable to set SDL OpenGl attribute");
    return false;
  }
  #ifdef _WIN32
    if (SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2) != 0) {
      BM_ERROR("Unable to set SDL OpenGl attribute");
      return false;
    }
    if (SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0) != 0) {
      BM_ERROR("Unable to set SDL OpenGl attribute");
      return false;
    }
  #endif
  if (SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24) != 0) {
    BM_ERROR("Unable to set SDL OpenGl attribute");
    return false;
  }

  // XXX[29.7.2012 alex]: casts size_t to int
  _sdl_surface = SDL_SetVideoMode(width, height, 24, videoFlags);
  if (!_sdl_surface) {
    BM_ERROR("Unable to set video mode.");
    return false;
  }

  if (fullscreen) {
    if(SDL_WM_ToggleFullScreen(_sdl_surface) != 1) {
      BM_ERROR("Unable to toggle fullscreen.");
      return false;
    }
  }

  return true;
}

bool RenderWindow::initGLEW() {
  glewExperimental = GL_TRUE;
  GLenum error = glewInit();
  if (error != GLEW_OK) {
    BM_ERROR("Unable to initalize GLEW.");
    return false;
  }
  return true;
}

void RenderWindow::resizeViewport(int w, int h) {
  glViewport(0, 0, static_cast<GLsizei>(w), static_cast<GLsizei>(h));
}

void RenderWindow::enable2D() {
  // Get viewport size.
  GLint viewport[4];
  glGetIntegerv(GL_VIEWPORT, viewport);

  // Set orthogonal projection matrix.
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(viewport[0], viewport[0] + viewport[2],
    viewport[1] + viewport[3], viewport[1], -1, 1);

  // Set model-view matrix to the identity.
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  // Enable alpha blending.
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // Enable 2d texturing.
  glEnable(GL_TEXTURE_2D);
}

bool RenderWindow::initOpenGL(int width, int height) {
  resizeViewport(width, height);
  enable2D();

  if (glGetError() != GL_NO_ERROR) {
    BM_ERROR("Unable to initalize OpenGL.");
    return false;
  }

  return true;
}

} // namespace bm
