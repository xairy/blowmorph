#ifndef BLOWMORPH_CLIENT_RENDER_WINDOW_HPP_
#define BLOWMORPH_CLIENT_RENDER_WINDOW_HPP_

#include <GL/glew.h>
#include <SDL/SDL.h>

namespace bm {

class RenderWindow {
public:
  RenderWindow();

  bool Init(const char* title, size_t width, size_t height, bool fullscreen);
  void Finalize();
  void SwapBuffers();

private:
  bool initSDL(const char* title, size_t width, size_t height, bool fullscreen);
  bool initGLEW();
  void resizeViewport(size_t w, size_t h);
  void enable2D();
  bool initOpenGL(size_t width, size_t height);

  enum State {
    STATE_FINALIZED,
    STATE_INITIALIZED
  };

  State _state;
  SDL_Surface* _sdl_surface;
};

} // namespace bm

#endif // BLOWMORPH_CLIENT_RENDER_WINDOW_HPP_
