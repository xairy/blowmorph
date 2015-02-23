// Copyright (c) 2015 Blowmorph Team

#ifndef ENGINE_CONFIG_H_
#define ENGINE_CONFIG_H_

#include <map>
#include <string>
#include <vector>

#include "base/config_reader.h"
#include "base/macros.h"
#include "base/pstdint.h"
#include "base/singleton.h"

namespace bm {

class Config : public Singleton<Config> {
 public:
  struct MasterServerConfig {
    uint16_t port;
  };

  struct ServerConfig {
    uint16_t port;
    int32_t tick_rate;
    int32_t broadcast_rate;
    std::string map;
    std::string name;

    std::string master_server_host;
    uint16_t master_server_port;
  };

  struct ClientConfig {
    std::string server_host;
    uint16_t server_port;

    std::string master_server_host;
    uint16_t master_server_port;

    std::string player_name;

    int32_t screen_width;
    int32_t screen_height;
    bool fullscreen;

    int32_t tick_rate;
    int32_t connect_timeout;
    int32_t sync_timeout;
    float32_t max_player_misposition;  // FIXME(xairy): rename.
    int32_t interpolation_offset;
  };

  struct BodyConfig {
    enum ShapeType {
      SHAPE_TYPE_BOX,
      SHAPE_TYPE_CIRCLE,
    };

    struct BoxConfig {
      float32_t width;
      float32_t height;
    };

    struct CircleConfig {
      float32_t radius;
    };

    std::string name;
    bool dynamic;
    ShapeType shape_type;
    BoxConfig box_config;
    CircleConfig circle_config;
  };

  struct TextureConfig {
    std::string name;
    std::string image;
    uint32_t transparent_color;

    bool tiled;
    int32_t tile_start_x, tile_start_y;
    int32_t tile_step_x, tile_step_y;
    int32_t tile_width, tile_height;
  };

  struct SpriteConfig {
    struct Mode {
      std::vector<int32_t> tiles;
      int64_t timeout;
      bool cyclic;
    };

    std::string name;
    std::string texture_name;

    bool has_modes;
    Mode mode;  // TODO(xairy): support multiple modes.
  };

  struct ActivatorConfig {
    enum Type {
      TYPE_DOOR,
    };

    std::string name;
    std::string body_name;
    std::string sprite_name;
    float32_t activation_distance;
    Type type;
  };

  struct CritterConfig {
    enum Type {
      TYPE_ZOMBIE,
    };

    std::string name;
    std::string body_name;
    std::string sprite_name;
    float32_t speed;
    int32_t damage;
    Type type;
  };

  struct KitConfig {
    enum Type {
      TYPE_HEALTH,
      TYPE_ENERGY,
      TYPE_COMPOSITE,
    };

    std::string name;
    std::string body_name;
    std::string sprite_name;
    Type type;

    // TODO(xairy): regeneration.
  };

  struct PlayerConfig {
    // TODO(xairy): types.

    std::string name;
    std::string body_name;
    std::string sprite_name;
    float32_t speed;
    int32_t health_max;
    int32_t health_regen;
    int32_t energy_max;
    int32_t energy_regen;
  };

  struct ProjectileConfig {
    enum Type {
      TYPE_ROCKET,
      TYPE_SLIME,
    };

    struct RocketConfig {
      float32_t explosion_radius;
      int32_t explosion_damage;
    };

    struct SlimeConfig {
      int32_t explosion_radius;
    };

    std::string name;
    std::string body_name;
    std::string sprite_name;
    float32_t speed;
    Type type;
    RocketConfig rocket_config;
    SlimeConfig slime_config;
  };

  struct WallConfig {
    enum Type {
      TYPE_ORDINARY,
      TYPE_UNBREAKABLE,
      TYPE_MORPHED,
    };

    std::string name;
    std::string body_name;
    std::string sprite_name;
    Type type;
  };

  struct GunConfig {
    std::string name;
    std::string projectile_name;
    int32_t energy_consumption;
  };

  Config();
  ~Config();

  bool Initialize();

  const MasterServerConfig& GetMasterServerConfig() const;
  const ServerConfig& GetServerConfig() const;
  const ClientConfig& GetClientConfig() const;

  const std::map<std::string, BodyConfig>& GetBodiesConfig() const;
  const std::map<std::string, TextureConfig>& GetTexturesConfig() const;
  const std::map<std::string, SpriteConfig>& GetSpritesConfig() const;

  const std::map<std::string, ActivatorConfig>& GetActivatorsConfig() const;
  const std::map<std::string, CritterConfig>& GetCrittersConfig() const;
  const std::map<std::string, KitConfig>& GetKitsConfig() const;
  const std::map<std::string, PlayerConfig>& GetPlayersConfig() const;
  const std::map<std::string, ProjectileConfig>& GetProjectilesConfig() const;
  const std::map<std::string, WallConfig>& GetWallsConfig() const;

  const std::map<std::string, GunConfig>& GetGunsConfig() const;

 private:
  bool LoadMasterServerConfig();
  bool LoadServerConfig();
  bool LoadClientConfig();

  bool LoadBodiesConfig();
  bool LoadTexturesConfig();
  bool LoadSpritesConfig();

  bool LoadActivatorsConfig();
  bool LoadCrittersConfig();
  bool LoadKitsConfig();
  bool LoadPlayersConfig();
  bool LoadProjectilesConfig();
  bool LoadWallsConfig();

  bool LoadGunsConfig();

  MasterServerConfig master_server_;
  ServerConfig server_;
  ClientConfig client_;

  std::map<std::string, BodyConfig> bodies_;
  std::map<std::string, TextureConfig> textures_;
  std::map<std::string, SpriteConfig> sprites_;

  std::map<std::string, ActivatorConfig> activators_;
  std::map<std::string, CritterConfig> critters_;
  std::map<std::string, KitConfig> kits_;
  std::map<std::string, PlayerConfig> players_;
  std::map<std::string, ProjectileConfig> projectiles_;
  std::map<std::string, WallConfig> walls_;

  std::map<std::string, GunConfig> guns_;

  enum {
    STATE_FINALIZED,
    STATE_INITIALIZED
  } state_;

  DISALLOW_COPY_AND_ASSIGN(Config);
};

}  // namespace bm

#endif  // ENGINE_CONFIG_H_
