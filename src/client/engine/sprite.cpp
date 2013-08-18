#include "sprite.hpp"

#include <vector>

#include <SFML/Graphics.hpp>

#include <base/error.hpp>
#include <base/pstdint.hpp>
#include <base/timer.hpp>

#include "texture_atlas.hpp"

namespace bm {

Sprite::Sprite() : _state(STATE_FINALIZED) { }
Sprite::~Sprite() {
  Finalize();
}

bool Sprite::Initialize(
  TextureAtlas* texture,
  int64_t timeout,
  bool animated,
  bool cyclic
) {
  CHECK(_state == STATE_FINALIZED);
  CHECK(texture != NULL);
  CHECK(texture->GetTileCount() > 0);
  
  _texture = texture;
  _timeout = timeout;
  _cyclic = cyclic;
  _animated = animated;
  _current_frame = 0;
  _state = STATE_STOPPED;

  _frame_count = _texture->GetTileCount();
  _frames.resize(_frame_count, NULL);
  for (size_t frame = 0; frame < _frame_count; frame++) {
    glm::vec2 tile_position = texture->GetTilePosition(frame);
    glm::vec2 tile_size = texture->GetTileSize(frame);

    sf::Sprite* sprite = new sf::Sprite();
    CHECK(sprite != NULL);
    sprite->setTexture(*texture->GetTexture());
    sprite->setTextureRect(sf::IntRect(tile_position.x, tile_position.y,
      tile_size.x, tile_size.y));
    sprite->setOrigin(tile_size.x / 2.0f, tile_size.y / 2.0f);

    _frames[frame] = sprite;
  }

  return true;
}

void Sprite::Finalize() {
  CHECK(_state == STATE_PLAYING || _state == STATE_STOPPED);
  for (size_t frame = 0; frame < _frame_count; frame++) {
    if (_frames[frame] != NULL) {
      delete _frames[frame];
      _frames[frame] = NULL;
    }
  }
  _state = STATE_FINALIZED;
}

void Sprite::Render(sf::RenderWindow& render_window) {
  CHECK(_state == STATE_PLAYING || _state == STATE_STOPPED);
  DCHECK(_frames[_current_frame] != NULL);
  if (_state == STATE_PLAYING) {
    updateCurrentFrame();
  }
  render_window.draw(*_frames[_current_frame]);
}

void Sprite::Play() {
  CHECK(_state == STATE_STOPPED);
  _last_frame_change = _timer.GetTime();
  _state = STATE_PLAYING;
}

void Sprite::Stop() {
  CHECK(_state == STATE_PLAYING);
  _state = STATE_STOPPED;
}

size_t Sprite::GetCurrentFrame() const {
  return _current_frame;
}

void Sprite::SetCurrentFrame(size_t frame) {
  _current_frame = frame;
}

bool Sprite::IsPlaying() const {
  return _state == STATE_PLAYING;
}

bool Sprite::IsStopped() const {
  return _state == STATE_STOPPED;
}

void Sprite::SetPosition(const glm::vec2& position) {
  CHECK(_state == STATE_PLAYING || _state == STATE_STOPPED);
  for (size_t frame = 0; frame < _frame_count; frame++) {
    DCHECK(_frames[frame] != NULL);
    _frames[frame]->setPosition(sf::Vector2f(position.x, position.y));
  }
}

glm::vec2 Sprite::GetPosition() const {
  CHECK(_state == STATE_PLAYING || _state == STATE_STOPPED);
  DCHECK(_frames[_current_frame] != NULL);
  sf::Vector2f position = _frames[_current_frame]->getPosition();
  return glm::vec2(position.x, position.y);
}

void Sprite::SetPivot(const glm::vec2& pivot) {
  CHECK(_state == STATE_PLAYING || _state == STATE_STOPPED);
  for (size_t frame = 0; frame < _frame_count; frame++) {
    DCHECK(_frames[frame] != NULL);
    _frames[frame]->setOrigin(pivot.x, pivot.y);
  }
}

glm::vec2 Sprite::GetPivot() const {
  CHECK(_state == STATE_PLAYING || _state == STATE_STOPPED);
  DCHECK(_frames[_current_frame] != NULL);
  sf::Vector2f pivot = _frames[_current_frame]->getOrigin();
  return glm::vec2(pivot.x, pivot.y);
}

void Sprite::updateCurrentFrame() {
  CHECK(_state == STATE_PLAYING || _state == STATE_STOPPED);
  if (!_animated) {
    return;
  }
  int64_t current_time = _timer.GetTime();
  if (current_time >= _last_frame_change + _timeout) {
    _last_frame_change = current_time;
    DCHECK(_frame_count > 0);
    if (_current_frame == _frame_count - 1) {
      if (!_cyclic) {
        Stop();
      } else {
        _current_frame = 0;
      }
    } else {
      _current_frame = _current_frame + 1;
    }
  }
}

} // namespace bm
