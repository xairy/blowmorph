// Copyright (c) 2013 Blowmorph Team

#include "client/resource_manager.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "base/macros.h"
#include "base/settings_manager.h"

#include "client/sprite.h"
#include "client/texture_atlas.h"

namespace bm {

ResourceManager::ResourceManager() {
  bool rv = textures_settings_.Open("data/textures.cfg");
  CHECK(rv == true);

  rv = sprites_settings_.Open("data/sprites.cfg");
  CHECK(rv == true);
}

ResourceManager::~ResourceManager() {
  std::map<std::string, TextureAtlas*>::iterator i;
  for (i = textures_.begin(); i != textures_.end(); ++i) {
    delete i->second;
  }
}

Sprite* ResourceManager::CreateSprite(const std::string& id) {
  std::string texture_id = sprites_settings_.GetString(id + ".texture");
  TextureAtlas* texture = LoadTexture(texture_id);
  if (texture == NULL) {
    return NULL;
  }

  std::vector<int32_t> tiles;
  int64_t timeout = 0;
  bool cyclic = false;

  sprites_settings_.LookupInt32List(id + ".mode.tiles", &tiles);
  sprites_settings_.LookupInt64(id + ".mode.timeout", &timeout);
  sprites_settings_.LookupBool(id + ".mode.cyclic", &cyclic);

  Sprite* sprite = new Sprite();
  CHECK(sprite != NULL);
  sprite->Initialize(texture, tiles, timeout, cyclic);
  return sprite;
}

TextureAtlas* ResourceManager::LoadTexture(const std::string& id) {
  if (textures_.count(id) != 0) {
    return textures_[id];
  }

  std::string image = textures_settings_.GetString(id + ".image");

  uint32_t transparent_color;
  bool rv = textures_settings_.LookupUInt32(id + ".transparent_color",
                                            &transparent_color);
  if (rv == false) {
    transparent_color = 0xFFFFFFFF;
  }

  bool tiled = false;
  if (textures_settings_.HasSetting(id + ".tile")) {
    tiled = true;
  }

  std::auto_ptr<TextureAtlas> texture(new TextureAtlas());
  CHECK(texture.get() != NULL);

  if (tiled) {
    int32_t start_x = textures_settings_.GetInt32(id + ".tile.start.x");
    int32_t start_y = textures_settings_.GetInt32(id + ".tile.start.y");
    int32_t horizontal_step =
      textures_settings_.GetInt32(id + ".tile.step.horizontal");
    int32_t vertical_step =
      textures_settings_.GetInt32(id + ".tile.step.vertical");
    int32_t width = textures_settings_.GetInt32(id + ".tile.width");
    int32_t height = textures_settings_.GetInt32(id + ".tile.height");
    bool rv = texture->LoadTileset(image, transparent_color, start_x, start_y,
        horizontal_step, vertical_step, width, height);
    if (rv == false) {
      return NULL;
    }
  } else {
    bool rv = texture->LoadTexture(image, transparent_color);
    if (rv == false) {
      return NULL;
    }
  }

  CHECK(texture->GetTileCount() > 0);

  textures_[id] = texture.get();
  return texture.release();
}

}  // namespace bm
