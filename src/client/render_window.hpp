#ifndef BLOWMORPH_CLIENT_RENDER_WINDOW_HPP_
#define BLOWMORPH_CLIENT_RENDER_WINDOW_HPP_

#include <GL/glew.h>
#include <SDL/SDL.h>

namespace bm {

class RenderWindow {
public:
  RenderWindow();
  ~RenderWindow();

  bool Initialize(const char* title, int width, int height, bool fullscreen);
  void Finalize();
  void SwapBuffers();

private:
  bool initSDL(const char* title, int width, int height, bool fullscreen);
  bool initGLEW();
  void resizeViewport(int w, int h);
  void enable2D();
  bool initOpenGL(int width, int height);

  enum State {
    STATE_FINALIZED,
    STATE_INITIALIZED
  };

  State _state;
  SDL_Surface* _sdl_surface;
};

} // namespace bm

#endif // BLOWMORPH_CLIENT_RENDER_WINDOW_HPP_
