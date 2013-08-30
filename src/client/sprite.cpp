#include "sprite.hpp"

#include <map>
#include <string>
#include <vector>

#include <SFML/Graphics.hpp>

#include <base/error.hpp>
#include <base/pstdint.hpp>
#include <base/settings_manager.hpp>
#include <base/timer.hpp>

#include "texture_atlas.hpp"

namespace bm {

Sprite::Sprite() : _state(STATE_FINALIZED) { }

bool Sprite::Initialize(const std::string& path) {
  SettingsManager settings;
  if (!settings.Open(path)) {
    return false;
  }

  std::string source;
  bool rv = settings.LookupString("sprite.source", &source);
  CHECK(rv == true);

  uint32_t transparent_color;
  rv = settings.LookupUInt32("sprite.transparent_color", &transparent_color);
  CHECK(rv == true);

  bool tiled = false;
  if (settings.HasSetting("sprite.tile")) {
    tiled = true;
  }

  // FIXME(xairy): texture shouldn't be loaded twice.
  if (tiled) {
    int32_t start_x, start_y;
    rv = settings.LookupInt32("sprite.tile.start.x", &start_x);
    CHECK(rv == true);
    rv = settings.LookupInt32("sprite.tile.start.y", &start_y);
    CHECK(rv == true);

    int32_t horizontal_step, vertical_step;
    rv = settings.LookupInt32("sprite.tile.step.horizontal", &horizontal_step);
    CHECK(rv == true);
    rv = settings.LookupInt32("sprite.tile.step.vertical", &vertical_step);
    CHECK(rv == true);

    int32_t width, height;
    rv = settings.LookupInt32("sprite.tile.width", &width);
    CHECK(rv == true);
    rv = settings.LookupInt32("sprite.tile.height", &height);
    CHECK(rv == true);

    // XXX(xairy): WTF size_t?.
    _texture = LoadTileset(source, transparent_color, start_x, start_y,
        horizontal_step, vertical_step, width, height);
  } else {
    _texture = LoadOldTexture(source, transparent_color);
  }
  if (_texture == NULL) {
    return false;
  }

  CHECK(_texture->GetTileCount() > 0);

  // TODO(xairy): support multiple modes.
  // TODO(xairy): support tile numbers.

  settings.LookupInt64("sprite.mode.timeout", &_current_mode.timeout);
  settings.LookupBool("sprite.mode.cyclic", &_current_mode.cyclic);

  _frames_count = _texture->GetTileCount();
  _frames.resize(_frames_count, NULL);
  CHECK(_frames_count >= 1);

  for (size_t frame = 0; frame < _frames_count; frame++) {
    glm::vec2 tile_position = _texture->GetTilePosition(frame);
    glm::vec2 tile_size = _texture->GetTileSize(frame);

    sf::Sprite* sprite = new sf::Sprite();
    CHECK(sprite != NULL);

    sprite->setTexture(*_texture->GetTexture());
    sprite->setTextureRect(sf::IntRect(tile_position.x, tile_position.y,
      tile_size.x, tile_size.y));
    sprite->setOrigin(tile_size.x / 2.0f, tile_size.y / 2.0f);

    _frames[frame] = sprite;
  }

  _current_frame = 0;
  _last_frame_change = _timer.GetTime();

  _state = STATE_STOPPED;
  return true;
}

Sprite::~Sprite() {
  Finalize();
}

void Sprite::Finalize() {
  CHECK(_state == STATE_PLAYING || _state == STATE_STOPPED);
  for (size_t frame = 0; frame < _frames_count; frame++) {
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
  UpdateCurrentFrame();
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
  for (size_t frame = 0; frame < _frames_count; frame++) {
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
  for (size_t frame = 0; frame < _frames_count; frame++) {
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

void Sprite::UpdateCurrentFrame() {
  CHECK(_state == STATE_PLAYING || _state == STATE_STOPPED);
  if (_state == STATE_STOPPED || _frames_count == 1) {
    return;
  }
  int64_t current_time = _timer.GetTime();
  if (current_time >= _last_frame_change + _current_mode.timeout) {
    _last_frame_change = current_time;
    if (_current_frame == _frames_count - 1) {
      if (!_current_mode.cyclic) {
        Stop();
      } else {
        _current_frame = 0;
      }
    } else {
      _current_frame++;
    }
  }
}

} // namespace bm
