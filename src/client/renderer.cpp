#include "renderer.hpp"

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

Renderer::Renderer() : _state(STATE_FINALIZED) { }
Renderer::~Renderer() {
  CHECK(_state == STATE_FINALIZED);
}

bool Renderer::Initialize(GameController* game_controller) {
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

void Renderer::Finalize() {
  CHECK(_state == STATE_INITIALIZED);

  _game_controller = NULL;
  _render_window.Finalize();

  _state = STATE_FINALIZED;
}

bool Renderer::Render() {
  _render_window.SwapBuffers();
  return true;
}

bool Renderer::PumpEvents() {
  SDL_Event event;
  while(SDL_PollEvent(&event)) {
    /*if(!_ProcessEvent(&event)) {
      return false;
    }*/
  }
  return true;
}

} // namespace bm
