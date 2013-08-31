#ifndef BLOWMORPH_CLIENT_SPRITE_H_
#define BLOWMORPH_CLIENT_SPRITE_H_

#include <string>
#include <vector>

#include <SFML/Graphics.hpp>
#include <glm/glm.hpp>

#include <base/pstdint.h>
#include <base/timer.h>

namespace bm {

class TextureAtlas;

class Sprite {
public:
  Sprite();
  ~Sprite();

  // Initializes 'Sprite'. 'path' is the path to a sprite description file.
  // Returns 'true' on success, returns 'false' on error.
  bool Initialize(const std::string& path);

  // Cleans up. Automatically called in the destructor.
  void Finalize();

  // Renders current frame.
  void Render(sf::RenderWindow& render_window);

  // Sets the mode of the sprite. 'mode' should be the name of one of the
  // modes declared in the sprite description file.
  // void SetMode(const std::string& mode);

  // Starts changing frames over time.
  // Does nothing for one-frames modes.
  void Play();

  // Stops changing frames over time.
  // Does nothing for one-frame modes.
  void Stop();

  size_t GetCurrentFrame() const;
  void SetCurrentFrame(size_t frame);

  bool IsPlaying() const;
  bool IsStopped() const;

  void SetPosition(const glm::vec2& value);
  glm::vec2 GetPosition() const;

  // Setter and getter for pivot (a point relative to
  // the texture to which sprite is attached).
  void SetPivot(const glm::vec2& value);
  glm::vec2 GetPivot() const;

private:
  void UpdateCurrentFrame();

  enum {
    STATE_FINALIZED,
    STATE_STOPPED,
    STATE_PLAYING
  } _state;

  struct Mode {
    // std::string name;
    int64_t timeout;
    bool cyclic;
    // std::vector<size_t> tiles;
  };

  TextureAtlas* _texture;

  Mode _current_mode;

  std::vector<sf::Sprite*> _frames;
  size_t _frames_count;
  size_t _current_frame;

  Timer _timer;
  int64_t _last_frame_change;
};

} // namespace bm

#endif
