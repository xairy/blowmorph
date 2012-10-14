#ifndef BLOWMORPH_CLIENT_WINDOW_HPP_
#define BLOWMORPH_CLIENT_WINDOW_HPP_

#include <base/macros.hpp>
#include <base/pstdint.hpp>

//#include "engine/animation.hpp"
#include "engine/canvas.hpp"
#include "engine/render_window.hpp"
//#include "engine/sprite.hpp"
//#include "engine/texture_atlas.hpp"
//#include "engine/text_writer.hpp"

namespace bm {

class GameController;

class Window {
public:
  Window();
  ~Window();

  bool Initialize(GameController* game_controller);
  void Finalize();

  bool Render();
  bool PumpEvents();

private:
  DISALLOW_COPY_AND_ASSIGN(Window);

  GameController* _game_controller;

  RenderWindow _render_window;
  Canvas _canvas;

  enum {
    STATE_FINALIZED,
    STATE_INITIALIZED
  } _state;
};

} // namespace bm

#endif // BLOWMORPH_CLIENT_WINDOW_HPP_
