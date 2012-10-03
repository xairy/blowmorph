#include "window.hpp"

#include <SDL/SDL.h>

#include <base/error.hpp>

//#include "engine/animation.hpp"
#include "engine/canvas.hpp"
#include "engine/render_window.hpp"
//#include "engine/sprite.hpp"
//#include "engine/texture_atlas.hpp"
//#include "engine/text_writer.hpp"

#include "game_controller.hpp"

namespace bm {

Window::Window() : _state(STATE_FINALIZED) { }
Window::~Window() {
  CHECK(_state == STATE_FINALIZED);
}

bool Window::Initialize(GameController* game_controller) {
  CHECK(_state == STATE_FINALIZED);

  if (!_render_window.Initialize("Blowmorph", 600, 600, false)) {
    BM_ERROR("Could not initialize render window!");
    return false;
  }
  
  if (!_canvas.Init()) {
    BM_ERROR("Could not initialize canvas!");
    return false;
  }

  _game_controller = game_controller;

  _state = STATE_INITIALIZED;
  return true;
}

void Window::Finalize() {
  CHECK(_state == STATE_INITIALIZED);

  _game_controller = NULL;
  _render_window.Finalize();

  _state = STATE_FINALIZED;
}

bool Window::Render() {
  _render_window.SwapBuffers();
  return true;
}

bool Window::PumpEvents() {
  SDL_Event event;
  while(SDL_PollEvent(&event)) {
    /*if(!_ProcessEvent(&event)) {
      return false;
    }*/
  }
  return true;
}

} // namespace bm
