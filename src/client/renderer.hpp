#ifndef BLOWMORPH_CLIENT_RENDERER_HPP_
#define BLOWMORPH_CLIENT_RENDERER_HPP_

#include <base/macros.hpp>

//#include "engine/animation.hpp"
#include "engine/canvas.hpp"
#include "engine/render_window.hpp"
//#include "engine/sprite.hpp"
//#include "engine/texture_atlas.hpp"
//#include "engine/text_writer.hpp"

namespace bm {

class GameController;

// XXX[xairy, 03.10.2012]: also it handles input and other SDL events?
class Renderer {
public:
  Renderer();
  ~Renderer();

  bool Initialize(GameController* game_controller);
  void Finalize();

  bool Render();

  bool PumpEvents();

private:
  DISALLOW_COPY_AND_ASSIGN(Renderer);

  GameController* _game_controller;

  RenderWindow _render_window;
  Canvas _canvas;

  enum {
    STATE_FINALIZED,
    STATE_INITIALIZED
  } _state;
};

} // namespace bm

#endif // BLOWMORPH_CLIENT_RENDERER_HPP_
