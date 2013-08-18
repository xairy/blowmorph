#ifndef BLOWMORPH_CLIENT_SPRITE_HPP__
#define BLOWMORPH_CLIENT_SPRITE_HPP_

#include <vector>

#include <SFML/Graphics.hpp>
#include <glm/glm.hpp>

#include <base/pstdint.hpp>
#include <base/timer.hpp>

namespace bm {

class TextureAtlas;

class Sprite {
public:
  Sprite();
  ~Sprite();

  // Initializes sprite. 'texture' is a tiled texture, tiles are used
  // as frames. 'timeout' is a delay between frames in ms. 0th frame is
  // set to render initially. If 'cyclic' is set to 'true' animation will
  // be playing cyclically.
  // Returns 'true' on success, returns 'false' on error.
  // FIXME: comment.
  bool Initialize(
    TextureAtlas* texture,
    int64_t timeout,
    bool animated,
    bool cyclic
  );

  // Cleans up. Automatically called in '~Animation()'.
  void Finalize();

  // Renders current frame.
  void Render(sf::RenderWindow& render_window);

  // Starts changing frames over time.
  void Play();

  // Stops changing frames over time.
  void Stop();

  size_t GetCurrentFrame() const;
  void SetCurrentFrame(size_t frame);

  bool IsPlaying() const;
  bool IsStopped() const;

  // Sprite inherited methods.

  // Setter and getter for position.
  void SetPosition(const glm::vec2& value);
  glm::vec2 GetPosition() const;

  // Setter and getter for pivot (a point relative to
  // the texture to which sprite is attached).
  void SetPivot(const glm::vec2& value);
  glm::vec2 GetPivot() const;

private:
  void updateCurrentFrame();

  enum {
    STATE_FINALIZED,
    STATE_STOPPED,
    STATE_PLAYING
  } _state;

  TextureAtlas* _texture;
  int64_t _timeout;

  bool _animated;
  bool _cyclic;

  size_t _current_frame;
  size_t _frame_count;

  // XXX[12.08.2012 xairy]: use external timer?
  Timer _timer;
  int64_t _last_frame_change;

  std::vector<sf::Sprite*> _frames;
};

} // namespace bm

#endif
