#ifndef BLOWMORPH_CLIENT_ANIMATION_HPP_
#define BLOWMORPH_CLIENT_ANIMATION_HPP_

#include <vector>

#include <glm/glm.hpp>

#include <base/pstdint.hpp>
#include <base/timer.hpp>

namespace bm {

class Texture;
class Sprite;

class Animation {
public:
  Animation();
  ~Animation();

  // Initializes animation. 'texture' is a tiled texture, tiles are used
  // as frames. 'timeout' is a delay between frames in ms. 0th frame is
  // set to render initially.
  // Returns 'true' on success, returns 'false' on error.
  bool Initialize(Texture* texture, uint32_t timeout);

  // Cleans up. Automatically called in '~Animation()'.
  void Finalize();

  // Renders current frame using OpenGL API.
  void Render();

  // Starts changing frames over time.
  void Play();

  // Stops changing frames over time.
  void Stop();

  // Returns current frame.
  size_t GetCurrentFrame() const;

  // Sprite inherited methods.

  // Setter and getter for position.
  void SetPosition(const glm::vec2& value);
  glm::vec2 GetPosition() const;

private:
  void updateCurrentFrame();

  enum {
    STATE_FINALIZED,
    STATE_STOPPED,
    STATE_PLAYING
  } _state;

  Texture* _texture;
  uint32_t _timeout;

  size_t _current_frame;
  size_t _frame_count;

  // XXX[12.08.2012 xairy]: use external timer?
  Timer _timer;
  uint32_t _last_frame_change;

  // XXX[12.08.2012 xairy]: use opengl directly instead?
  std::vector<Sprite*> _frames;
};

} // namespace bm

#endif
