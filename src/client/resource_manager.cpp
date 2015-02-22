// Copyright (c) 2015 Blowmorph Team

#include "client/resource_manager.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "base/macros.h"

#include "engine/config.h"

#include "client/sprite.h"
#include "client/texture_atlas.h"

namespace bm {

ResourceManager::ResourceManager() { }

ResourceManager::~ResourceManager() {
  for (auto i = textures_.begin(); i != textures_.end(); ++i) {
    delete i->second;
  }
}

Sprite* ResourceManager::CreateSprite(const std::string& id) {
  if (Config::GetInstance()->GetSpritesConfig().count(id) == 0) {
    return NULL;
  }
  const Config::SpriteConfig& config =
    Config::GetInstance()->GetSpritesConfig().at(id);

  TextureAtlas* texture = LoadTexture(config.texture_name);
  if (texture == NULL) {
    return NULL;
  }

  Sprite* sprite = new Sprite();
  CHECK(sprite != NULL);
  sprite->Initialize(texture, config.mode.tiles,
      config.mode.timeout, config.mode.cyclic);

  return sprite;
}

TextureAtlas* ResourceManager::LoadTexture(const std::string& id) {
  if (textures_.count(id) != 0) {
    return textures_[id];
  }

  if (Config::GetInstance()->GetTexturesConfig().count(id) == 0) {
    return NULL;
  }
  const Config::TextureConfig& config =
    Config::GetInstance()->GetTexturesConfig().at(id);

  std::auto_ptr<TextureAtlas> texture(new TextureAtlas());
  CHECK(texture.get() != NULL);

  if (config.tiled) {
    bool rv = texture->LoadTileset(
        config.image, config.transparent_color,
        config.tile_start_x, config.tile_start_y,
        config.tile_step_x, config.tile_step_y,
        config.tile_width, config.tile_height);
    if (rv == false) {
      return NULL;
    }
  } else {
    bool rv = texture->LoadTexture(config.image, config.transparent_color);
    if (rv == false) {
      return NULL;
    }
  }

  CHECK(texture->GetTileCount() > 0);

  textures_[id] = texture.get();
  return texture.release();
}

}  // namespace bm
