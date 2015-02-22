// Copyright (c) 2015 Blowmorph Team

#ifndef CLIENT_SPRITE_H_
#define CLIENT_SPRITE_H_

#include <string>
#include <vector>

#include <SFML/Graphics.hpp>

#include "base/pstdint.h"
#include "base/timer.h"

namespace bm {

class TextureAtlas;

class Sprite {
 public:
  Sprite();
  ~Sprite();

  // Initializes 'Sprite'.
  // Returns 'true' on success, returns 'false' on error.
  void Initialize(TextureAtlas* texture, const std::vector<int32_t>& tiles,
      int64_t timeout, bool cyclic);

  // Cleans up. Automatically called in the destructor.
  void Finalize();

  // Renders current frame.
  void Render(sf::RenderWindow* render_window);

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

  void SetPosition(const sf::Vector2f& value);
  sf::Vector2f GetPosition() const;

  void SetRotation(float angle);
  float GetRotation() const;

  // Setter and getter for pivot (a point relative to
  // the texture to which sprite is attached).
  void SetPivot(const sf::Vector2f& value);
  sf::Vector2f GetPivot() const;

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
    std::vector<size_t> tiles;
  };

  TextureAtlas* _texture;

  Mode _current_mode;

  std::vector<sf::Sprite*> _frames;
  size_t _frames_count;
  size_t _current_frame;

  Timer _timer;
  int64_t _last_frame_change;
};

}  // namespace bm

#endif  // CLIENT_SPRITE_H_
