// Copyright (c) 2015 Blowmorph Team

#ifndef ENGINE_CONFIG_H_
#define ENGINE_CONFIG_H_

#include <map>
#include <string>
#include <vector>

#include "base/macros.h"
#include "base/pstdint.h"

#include "engine/dll.h"

namespace bm {

// FIXME(xairy): remove dynamic config from cfg files.

class Config {
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
      SHAPE_TYPE_POLYGON,
    };

    struct BoxConfig {
      float32_t width;
      float32_t height;
    };

    struct CircleConfig {
      float32_t radius;
    };

    struct PolygonConfig {
      struct Vertice {
        float32_t x, y;
      };
      std::vector<Vertice> vertices;
    };

    std::string name;
    bool dynamic;
    ShapeType shape_type;
    BoxConfig box_config;
    CircleConfig circle_config;
    PolygonConfig polygon_config;
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
      int32_t timeout;
      bool cyclic;
    };

    std::string name;
    std::string texture_name;

    bool has_modes;
    Mode mode;  // TODO(xairy): support multiple modes.
  };

  struct ActivatorConfig {
    std::string name;
    std::string body_name;
    std::string sprite_name;
    float32_t activation_distance;
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

  struct DoorConfig {
    std::string name;
    std::string body_name;
    std::string sprite_name;
    float32_t activation_distance;
  };

  struct KitConfig {
    std::string name;
    std::string body_name;
    std::string sprite_name;
    int32_t health_regen;
    int32_t energy_regen;
  };

  struct PlayerConfig {
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

  BM_ENGINE_DECL static Config* GetInstance();

  BM_ENGINE_DECL Config();
  BM_ENGINE_DECL ~Config();

  BM_ENGINE_DECL bool Initialize();

  BM_ENGINE_DECL const MasterServerConfig& GetMasterServerConfig() const;
  BM_ENGINE_DECL const ServerConfig& GetServerConfig() const;
  BM_ENGINE_DECL const ClientConfig& GetClientConfig() const;

  BM_ENGINE_DECL const std::map<std::string, BodyConfig>& GetBodiesConfig() const;
  BM_ENGINE_DECL const std::map<std::string, TextureConfig>& GetTexturesConfig() const;
  BM_ENGINE_DECL const std::map<std::string, SpriteConfig>& GetSpritesConfig() const;

  BM_ENGINE_DECL const std::map<std::string, ActivatorConfig>& GetActivatorsConfig() const;
  BM_ENGINE_DECL const std::map<std::string, CritterConfig>& GetCrittersConfig() const;
  BM_ENGINE_DECL const std::map<std::string, DoorConfig>& GetDoorsConfig() const;
  BM_ENGINE_DECL const std::map<std::string, KitConfig>& GetKitsConfig() const;
  BM_ENGINE_DECL const std::map<std::string, PlayerConfig>& GetPlayersConfig() const;
  BM_ENGINE_DECL const std::map<std::string, ProjectileConfig>& GetProjectilesConfig() const;
  BM_ENGINE_DECL const std::map<std::string, WallConfig>& GetWallsConfig() const;

  BM_ENGINE_DECL const std::map<std::string, GunConfig>& GetGunsConfig() const;

 private:
  bool LoadMasterServerConfig();
  bool LoadServerConfig();
  bool LoadClientConfig();

  bool LoadBodiesConfig();
  bool LoadTexturesConfig();
  bool LoadSpritesConfig();

  bool LoadActivatorsConfig();
  bool LoadCrittersConfig();
  bool LoadDoorsConfig();
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
  std::map<std::string, DoorConfig> doors_;
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
