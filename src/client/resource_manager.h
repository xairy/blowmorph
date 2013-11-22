// Copyright (c) 2013 Blowmorph Team

#ifndef CLIENT_RESOURCE_MANAGER_H_
#define CLIENT_RESOURCE_MANAGER_H_

#include <map>
#include <string>

#include "base/settings_manager.h"

#include "client/sprite.h"
#include "client/texture_atlas.h"

namespace bm {

class ResourceManager {
 public:
  ResourceManager();
  ~ResourceManager();

  Sprite* CreateSprite(const std::string& id);

 private:
  TextureAtlas* LoadTexture(const std::string& id);

  SettingsManager textures_settings_;
  SettingsManager sprites_settings_;

  std::map<std::string, TextureAtlas*> textures_;

  DISALLOW_COPY_AND_ASSIGN(ResourceManager);
};

}  // namespace bm

#endif  // CLIENT_RESOURCE_MANAGER_H_
