#include "render_window.hpp"

#include <base/macros.hpp>
#include <base/error.hpp>

namespace bm {

RenderWindow::RenderWindow() {
  _sdl_surface = NULL;
  _state = STATE_FINALIZED;
}

bool RenderWindow::Init(const char* title, size_t width, size_t height, bool fullscreen) {
  CHECK(_state == STATE_FINALIZED);

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

bool RenderWindow::initSDL(const char* title, size_t width, size_t height, bool fullscreen) {
  if(SDL_Init(SDL_INIT_VIDEO) < 0) {
    BM_ERROR("Unable to initialize SDL!");
    return false;
  }

  const SDL_VideoInfo *videoInfo = SDL_GetVideoInfo();
  if (!videoInfo) {
    BM_ERROR("Unable to get video info.");
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
  #ifdef _WIN32
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
  #endif
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

  // XXX[29.7.2012 alex]: casts size_t to int
  _sdl_surface = SDL_SetVideoMode(static_cast<int>(width), static_cast<int>(height), 24, videoFlags);
  if (!_sdl_surface) {
    BM_ERROR("Unable to set video mode.");
    return false;
  }

  SDL_WM_SetCaption(title, NULL);

  if (fullscreen) {
    SDL_WM_ToggleFullScreen(_sdl_surface);
  }

  return true;
}

bool RenderWindow::initGLEW() {
  glewExperimental = GL_TRUE;
  GLenum error = glewInit();
  if(error != GLEW_OK) {
    BM_ERROR("Could not initalize GLEW.");
    return false;
  }
  return true;
}

void RenderWindow::resizeViewport(size_t w, size_t h) {
  glViewport(0, 0, static_cast<GLsizei>(w), static_cast<GLsizei>(h));
}

void RenderWindow::enable2D() {
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
}

bool RenderWindow::initOpenGL(size_t width, size_t height) {
  resizeViewport(width, height);
  enable2D();

  // TODO: check for OpenGL errors.

  return true;
}

}; // namespace bm
