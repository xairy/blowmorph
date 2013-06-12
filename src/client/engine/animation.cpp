#include "animation.hpp"

#include <vector>

#include <base/error.hpp>
#include <base/pstdint.hpp>
#include <base/timer.hpp>

#include "sprite.hpp"
#include "texture_atlas.hpp"

namespace bm {

Animation::Animation() : _state(STATE_FINALIZED) { }
Animation::~Animation() {
  Finalize();
}

bool Animation::Initialize(TextureAtlas* texture, uint32_t timeout, bool cyclic) {
  CHECK(_state == STATE_FINALIZED);
  CHECK(texture != NULL);
  CHECK(texture->GetTileCount() > 0);
  
  _texture = texture;
  _timeout = timeout;
  _current_frame = 0;
  _cyclic = cyclic;
  _state = STATE_STOPPED;

  _frame_count = _texture->GetTileCount();
  _frames.resize(_frame_count, NULL);
  for(size_t frame = 0; frame < _frame_count; frame++) {
    Sprite* sprite = new Sprite();
    CHECK(sprite != NULL);
    _frames[frame] = sprite;
    if(!sprite->Init(texture, frame)) {
      return false;
    }
  }

  return true;
}

void Animation::Finalize() {
  CHECK(_state == STATE_PLAYING || _state == STATE_STOPPED);
  for(size_t frame = 0; frame < _frame_count; frame++) {
    if(_frames[frame] != NULL) {
      delete _frames[frame];
      _frames[frame] = NULL;
    }
  }
  _state = STATE_FINALIZED;
}

void Animation::Render(sf::RenderWindow& render_window) {
  CHECK(_state == STATE_PLAYING || _state == STATE_STOPPED);
  DCHECK(_frames[_current_frame] != NULL);
  if(_state == STATE_PLAYING) {
    updateCurrentFrame();
  }
  _frames[_current_frame]->Render(render_window);
}

void Animation::Play() {
  CHECK(_state == STATE_STOPPED);
  _last_frame_change = _timer.GetTime();
  _state = STATE_PLAYING;
}

void Animation::Stop() {
  CHECK(_state == STATE_PLAYING);
  _state = STATE_STOPPED;
}

size_t Animation::GetCurrentFrame() const {
  return _current_frame;
}

bool Animation::IsPlaying() const {
  return _state == STATE_PLAYING;
}
bool Animation::IsStopped() const {
  return _state == STATE_STOPPED;
}

void Animation::SetPosition(const glm::vec2& value) {
  CHECK(_state == STATE_PLAYING || _state == STATE_STOPPED);
  for(size_t frame = 0; frame < _frame_count; frame++) {
    DCHECK(_frames[frame] != NULL);
    _frames[frame]->SetPosition(value);
  }
}
glm::vec2 Animation::GetPosition() const {
  CHECK(_state == STATE_PLAYING || _state == STATE_STOPPED);
  DCHECK(_frames[_current_frame] != NULL);
  return _frames[_current_frame]->GetPosition();
}

void Animation::SetPivot(const glm::vec2& value) {
  CHECK(_state == STATE_PLAYING || _state == STATE_STOPPED);
  for(size_t frame = 0; frame < _frame_count; frame++) {
    DCHECK(_frames[frame] != NULL);
    _frames[frame]->SetPivot(value);
  }
}
glm::vec2 Animation::GetPivot() const {
  CHECK(_state == STATE_PLAYING || _state == STATE_STOPPED);
  DCHECK(_frames[_current_frame] != NULL);
  return _frames[_current_frame]->GetPivot();
}

void Animation::updateCurrentFrame() {
  CHECK(_state == STATE_PLAYING || _state == STATE_STOPPED);
  uint32_t current_time = _timer.GetTime();
  if(current_time >= _last_frame_change + _timeout) {
    _last_frame_change = current_time;
    DCHECK(_frame_count > 0);
    if(_current_frame == _frame_count - 1) {
      if(!_cyclic) {
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
