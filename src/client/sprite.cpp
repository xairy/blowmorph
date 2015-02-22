// Copyright (c) 2015 Blowmorph Team

#include "client/sprite.h"

#include <map>
#include <string>
#include <vector>

#include <SFML/Graphics.hpp>

#include "base/error.h"
#include "base/pstdint.h"
#include "base/timer.h"

#include "client/texture_atlas.h"

namespace bm {

Sprite::Sprite() : _state(STATE_FINALIZED) { }

void Sprite::Initialize(TextureAtlas* texture,
    const std::vector<int32_t>& tiles, int64_t timeout, bool cyclic) {
  CHECK(_state == STATE_FINALIZED);
  CHECK(texture != NULL);

  // TODO(xairy): support multiple modes.

  _texture = texture;
  _current_mode.timeout = timeout;
  _current_mode.cyclic = cyclic;

  if (!tiles.empty()) {
    for (size_t i = 0; i < tiles.size(); i++) {
      CHECK(tiles[i] >= 0);
      _current_mode.tiles.push_back(static_cast<size_t>(tiles[i]));
    }
  } else {
    size_t tile_count = _texture->GetTileCount();
    for (size_t tile = 0; tile < tile_count; tile++) {
      _current_mode.tiles.push_back(tile);
    }
  }

  _frames_count = _current_mode.tiles.size();
  _frames.resize(_frames_count, NULL);
  CHECK(_frames_count >= 1);

  for (size_t frame = 0; frame < _frames_count; frame++) {
    size_t tile = _current_mode.tiles[frame];
    sf::Vector2i tile_position = _texture->GetTilePosition(tile);
    sf::Vector2i tile_size = _texture->GetTileSize(tile);

    sf::Sprite* sprite = new sf::Sprite();
    CHECK(sprite != NULL);

    sprite->setTexture(*_texture->GetTexture());
    sprite->setTextureRect(sf::IntRect(tile_position, tile_size));
    sprite->setOrigin(tile_size.x / 2.0f, tile_size.y / 2.0f);

    _frames[frame] = sprite;
  }

  _current_frame = 0;
  _last_frame_change = _timer.GetTime();

  _state = STATE_STOPPED;
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

void Sprite::Render(sf::RenderWindow* render_window) {
  CHECK(_state == STATE_PLAYING || _state == STATE_STOPPED);
  DCHECK(_frames[_current_frame] != NULL);
  UpdateCurrentFrame();
  render_window->draw(*_frames[_current_frame]);
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

void Sprite::SetPosition(const sf::Vector2f& position) {
  CHECK(_state == STATE_PLAYING || _state == STATE_STOPPED);
  for (size_t frame = 0; frame < _frames_count; frame++) {
    DCHECK(_frames[frame] != NULL);
    _frames[frame]->setPosition(position);
  }
}

sf::Vector2f Sprite::GetPosition() const {
  CHECK(_state == STATE_PLAYING || _state == STATE_STOPPED);
  DCHECK(_frames[_current_frame] != NULL);
  return _frames[_current_frame]->getPosition();
}

void Sprite::SetRotation(float angle) {
  CHECK(_state == STATE_PLAYING || _state == STATE_STOPPED);
  for (size_t frame = 0; frame < _frames_count; frame++) {
    DCHECK(_frames[frame] != NULL);
    _frames[frame]->setRotation(angle);
  }
}

float Sprite::GetRotation() const {
  CHECK(_state == STATE_PLAYING || _state == STATE_STOPPED);
  DCHECK(_frames[_current_frame] != NULL);
  return _frames[_current_frame]->getRotation();
}

void Sprite::SetPivot(const sf::Vector2f& pivot) {
  CHECK(_state == STATE_PLAYING || _state == STATE_STOPPED);
  for (size_t frame = 0; frame < _frames_count; frame++) {
    DCHECK(_frames[frame] != NULL);
    _frames[frame]->setOrigin(pivot);
  }
}

sf::Vector2f Sprite::GetPivot() const {
  CHECK(_state == STATE_PLAYING || _state == STATE_STOPPED);
  DCHECK(_frames[_current_frame] != NULL);
  return _frames[_current_frame]->getOrigin();
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

}  // namespace bm
