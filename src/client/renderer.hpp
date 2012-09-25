#ifndef BLOWMORPH_CLIENT_RENDERER_HPP_
#define BLOWMORPH_CLIENT_RENDERER_HPP_

namespace bm {

class Renderer {
public:
  Renderer();
  ~Renderer();

  bool Initialize(GameController* game_controller);
  bool Finalize();

private:
  GameController* _game_controller;

  enum {
    STATE_FINALIZED,
    STATE_INITIALIZED
  } _state;
};

} // namespace bm

#endif // BLOWMORPH_CLIENT_RENDERER_HPP_
