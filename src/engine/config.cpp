// Copyright (c) 2015 Blowmorph Team

#include "engine/config.h"

#include <map>
#include <string>

#include "base/config_reader.h"
#include "base/error.h"
#include "base/macros.h"
#include "base/pstdint.h"
#include "base/singleton.h"

namespace bm {

Config::Config() : state_(STATE_FINALIZED) { }
Config::~Config() { }

bool Config::Initialize() {
	CHECK(state_ == STATE_FINALIZED);
	if (!LoadMasterServerConfig() || !LoadServerConfig() || !LoadClientConfig()) {
		return false;
	}
	if (!LoadBodiesConfig() || !LoadTexturesConfig() || !LoadSpritesConfig()) {
		return false;
	}
	if (!LoadActivatorsConfig() || !LoadCrittersConfig() || !LoadKitsConfig() ||
			!LoadPlayersConfig() || !LoadProjectilesConfig() || !LoadWallsConfig()) {
		return false;
	}
	if (!LoadGunsConfig()) {
		return false;
	}
	state_ = STATE_INITIALIZED;
	return true;
}

const Config::MasterServerConfig& Config::GetMasterServerConfig() const {
	CHECK(state_ == STATE_INITIALIZED);
	return master_server_;
}

const Config::ServerConfig& Config::GetServerConfig() const {
	CHECK(state_ == STATE_INITIALIZED);
	return server_;
}

const Config::ClientConfig& Config::GetClientConfig() const {
	CHECK(state_ == STATE_INITIALIZED);
	return client_;
}

const std::map<std::string, Config::BodyConfig>&
Config::GetBodiesConfig() const {
	CHECK(state_ == STATE_INITIALIZED);
	return bodies_;
}

const std::map<std::string, Config::TextureConfig>&
Config::GetTexturesConfig() const {
	CHECK(state_ == STATE_INITIALIZED);
	return textures_;
}

const std::map<std::string, Config::SpriteConfig>&
Config::GetSpritesConfig() const {
	CHECK(state_ == STATE_INITIALIZED);
	return sprites_;
}

const std::map<std::string, Config::ActivatorConfig>&
Config::GetActivatorsConfig() const {
	CHECK(state_ == STATE_INITIALIZED);
	return activators_;
}

const std::map<std::string, Config::CritterConfig>&
Config::GetCrittersConfig() const {
	CHECK(state_ == STATE_INITIALIZED);
	return critters_;
}

const std::map<std::string, Config::KitConfig>&
Config::GetKitsConfig() const {
	CHECK(state_ == STATE_INITIALIZED);
	return kits_;
}

const std::map<std::string, Config::PlayerConfig>&
Config::GetPlayersConfig() const {
	CHECK(state_ == STATE_INITIALIZED);
	return players_;
}

const std::map<std::string, Config::ProjectileConfig>&
Config::GetProjectilesConfig() const {
	CHECK(state_ == STATE_INITIALIZED);
	return projectiles_;
}

const std::map<std::string, Config::WallConfig>&
Config::GetWallsConfig() const {
	CHECK(state_ == STATE_INITIALIZED);
	return walls_;
}

const std::map<std::string, Config::GunConfig>&
Config::GetGunsConfig() const {
	CHECK(state_ == STATE_INITIALIZED);
	return guns_;
}

bool Config::LoadMasterServerConfig() {
	ConfigReader reader;
	const char* file = "data/master-server.cfg";
	if (!reader.Open(file)) {
		return false;
	}
	if (!reader.LookupUInt16("master-server.port", &master_server_.port)) {
		THROW_ERROR("Unable to load 'master-server.port' from '%s'.", file);
		return false;
	}
	return true;
}

bool Config::LoadServerConfig() {
	ConfigReader reader;
	const char* file = "data/server.cfg";
	if (!reader.Open(file)) {
		return false;
	}
	if (!reader.LookupUInt16("server.port", &server_.port)) {
		THROW_ERROR("Unable to load 'server.port' from '%s'.", file);
		return false;
	}
	if (!reader.LookupInt32("server.broadcast_rate", &server_.broadcast_rate)) {
		THROW_ERROR("Unable to load 'server.broadcast_rate' from '%s'.", file);
		return false;
	}
	if (!reader.LookupInt32("server.tick_rate", &server_.tick_rate)) {
		THROW_ERROR("Unable to load 'server.tick_rate' from '%s'.", file);
		return false;
	}
	if (!reader.LookupString("server.map", &server_.map)) {
		THROW_ERROR("Unable to load 'server.map' from '%s'.", file);
		return false;
	}
	if (!reader.LookupString("server.name", &server_.name)) {
		THROW_ERROR("Unable to load 'server.name' from '%s'.", file);
		return false;
	}
	if (!reader.LookupString("master-server.host", &server_.master_server_host)) {
		THROW_ERROR("Unable to load 'master-server.host' from '%s'.", file);
		return false;
	}
	if (!reader.LookupUInt16("master-server.port", &server_.master_server_port)) {
		THROW_ERROR("Unable to load 'master-server.port' from '%s'.", file);
		return false;
	}
	return true;
}

bool Config::LoadClientConfig() {
	ConfigReader reader;
	const char* file = "data/client.cfg";
	if (!reader.Open(file)) {
		return false;
	}
	if (!reader.LookupString("server.host", &client_.server_host)) {
		THROW_ERROR("Unable to load 'server.host' from '%s'.", file);
		return false;
	}
	if (!reader.LookupUInt16("server.port", &client_.server_port)) {
		THROW_ERROR("Unable to load 'server.port' from '%s'.", file);
		return false;
	}
	if (!reader.LookupString("master-server.host", &client_.master_server_host)) {
		THROW_ERROR("Unable to load 'master-server.host' from '%s'.", file);
		return false;
	}
	if (!reader.LookupUInt16("master-server.port", &client_.master_server_port)) {
		THROW_ERROR("Unable to load 'master-server.port' from '%s'.", file);
		return false;
	}
	if (!reader.LookupString("player.login", &client_.player_name)) {
		THROW_ERROR("Unable to load 'player.login' from '%s'.", file);
		return false;
	}
	if (!reader.LookupInt32("graphics.width", &client_.screen_width)) {
		THROW_ERROR("Unable to load 'graphics.width' from '%s'.", file);
		return false;
	}
	if (!reader.LookupInt32("graphics.height", &client_.screen_height)) {
		THROW_ERROR("Unable to load 'graphics.height' from '%s'.", file);
		return false;
	}
	if (!reader.LookupBool("graphics.fullscreen", &client_.fullscreen)) {
		THROW_ERROR("Unable to load 'fullscreen' from '%s'.", file);
		return false;
	}
	if (!reader.LookupInt32("client.tick_rate", &client_.tick_rate)) {
		THROW_ERROR("Unable to load 'client.tick_rate' from '%s'.", file);
		return false;
	}
	if (!reader.LookupInt32("client.connect_timeout", &client_.connect_timeout)) {
		THROW_ERROR("Unable to load 'client.connect_timeout' from '%s'.", file);
		return false;
	}
	if (!reader.LookupInt32("client.sync_timeout", &client_.sync_timeout)) {
		THROW_ERROR("Unable to load 'client.sync_timeout' from '%s'.", file);
		return false;
	}
	if (!reader.LookupFloat("client.max_player_misposition",
				&client_.max_player_misposition)) {
		THROW_ERROR("Unable to load 'client.max_player_misposition' from '%s'.",
								file);
		return false;
	}
	if (!reader.LookupInt32("client.interpolation_offset",
				&client_.interpolation_offset)) {
		THROW_ERROR("Unable to load 'client.interpolation_offset' from '%s'.",
								file);
		return false;
	}

	return true;
}

bool Config::LoadBodiesConfig() {
	ConfigReader reader;
	const char* file = "data/bodies.cfg";
	if (!reader.Open(file)) {
		return false;
	}
	std::vector<std::string> names;
	reader.GetRootConfigs(&names);
	for (auto name : names) {
		bodies_[name].name = name;
		std::string config = name + ".dynamic";
		if (!reader.LookupBool(config, &bodies_[name].dynamic)) {
			THROW_ERROR("Unable to load '%s' from '%s'.", config.c_str(), file);
			return false;
		}
		std::string type;
		config = name + ".shape.type";
		if (!reader.LookupString(config, &type)) {
			THROW_ERROR("Unable to load '%s' from '%s'.", config.c_str(), file);
			return false;
		}
		if (type == "box") {
			bodies_[name].shape_type = Config::BodyConfig::SHAPE_TYPE_BOX;
			config = name + ".shape.width";
			if (!reader.LookupFloat(config, &bodies_[name].box_config.width)) {
				THROW_ERROR("Unable to load '%s' from '%s'.", config.c_str(), file);
				return false;
			}
			config = name + ".shape.height";
			if (!reader.LookupFloat(config, &bodies_[name].box_config.height)) {
				THROW_ERROR("Unable to load '%s' from '%s'.", config.c_str(), file);
				return false;
			}
		} else if (type == "circle") {
			bodies_[name].shape_type = Config::BodyConfig::SHAPE_TYPE_CIRCLE;
			config = name + ".shape.radius";
			if (!reader.LookupFloat(config, &bodies_[name].circle_config.radius)) {
				THROW_ERROR("Unable to load '%s' from '%s'.", config.c_str(), file);
				return false;
			}
		} else {
			THROW_ERROR("Body shape type must be 'box' or 'circle'.");
			return false;
		}
	}
	return true;
}

bool Config::LoadTexturesConfig() {
	ConfigReader reader;
	const char* file = "data/textures.cfg";
	if (!reader.Open(file)) {
		return false;
	}
	std::vector<std::string> names;
	reader.GetRootConfigs(&names);
	for (auto name : names) {
		textures_[name].name = name;
		std::string config = name + ".image";
		if (!reader.LookupString(config, &textures_[name].image)) {
			THROW_ERROR("Unable to load '%s' from '%s'.", config.c_str(), file);
			return false;
		}
		config = name + ".transparent_color";
		if (!reader.LookupUInt32(config, &textures_[name].transparent_color)) {
			textures_[name].transparent_color = 0xFFFFFFFF;
		}
		textures_[name].tiled = reader.HasSetting(name + ".tile");
		if (!textures_[name].tiled) {
			continue;
		}
		config = name + ".tile.start.x";
		if (!reader.LookupInt32(config, &textures_[name].tile_start_x)) {
			THROW_ERROR("Unable to load '%s' from '%s'.", config.c_str(), file);
			return false;
		}
		config = name + ".tile.start.y";
		if (!reader.LookupInt32(config, &textures_[name].tile_start_y)) {
			THROW_ERROR("Unable to load '%s' from '%s'.", config.c_str(), file);
			return false;
		}
		config = name + ".tile.step.horizontal";
		if (!reader.LookupInt32(config, &textures_[name].tile_step_x)) {
			THROW_ERROR("Unable to load '%s' from '%s'.", config.c_str(), file);
			return false;
		}
		config = name + ".tile.step.vertical";
		if (!reader.LookupInt32(config, &textures_[name].tile_step_y)) {
			THROW_ERROR("Unable to load '%s' from '%s'.", config.c_str(), file);
			return false;
		}
		config = name + ".tile.width";
		if (!reader.LookupInt32(config, &textures_[name].tile_width)) {
			THROW_ERROR("Unable to load '%s' from '%s'.", config.c_str(), file);
			return false;
		}
		config = name + ".tile.height";
		if (!reader.LookupInt32(config, &textures_[name].tile_height)) {
			THROW_ERROR("Unable to load '%s' from '%s'.", config.c_str(), file);
			return false;
		}
	}
	return true;
}

bool Config::LoadSpritesConfig() {
	ConfigReader reader;
	const char* file = "data/sprites.cfg";
	if (!reader.Open(file)) {
		return false;
	}
	std::vector<std::string> names;
	reader.GetRootConfigs(&names);
	for (auto name : names) {
		sprites_[name].name = name;
		std::string config = name + ".texture";
		if (!reader.LookupString(config, &sprites_[name].texture_name)) {
			THROW_ERROR("Unable to load '%s' from '%s'.", config.c_str(), file);
			return false;
		}
		if (textures_.count(sprites_[name].texture_name) == 0) {
			THROW_ERROR("Texture '%s' used by sprite '%s' not defined.",
				sprites_[name].texture_name.c_str(), name.c_str());
			return false;
		}
		sprites_[name].has_modes = reader.HasSetting(name + ".mode");
		if (!sprites_[name].has_modes) {
			sprites_[name].mode.timeout = 0;
			sprites_[name].mode.cyclic = false;
			continue;
		}
		config = name + ".mode.tiles";
		if (!reader.LookupInt32List(config, &sprites_[name].mode.tiles)) {
			THROW_ERROR("Unable to load '%s' from '%s'.", config.c_str(), file);
			return false;
		}
		config = name + ".mode.timeout";
		if (!reader.LookupInt64(config, &sprites_[name].mode.timeout)) {
			sprites_[name].mode.timeout = 0;
		}
		config = name + ".mode.cyclic";
		if (!reader.LookupBool(config, &sprites_[name].mode.cyclic)) {
			sprites_[name].mode.cyclic = false;
		}
	}
	return true;
}

bool Config::LoadActivatorsConfig() {
	ConfigReader reader;
	const char* file = "data/activators.cfg";
	if (!reader.Open(file)) {
		return false;
	}
	std::vector<std::string> names;
	reader.GetRootConfigs(&names);
	for (auto name : names) {
		activators_[name].name = name;

		std::string config = name + ".body";
		if (!reader.LookupString(config, &activators_[name].body_name)) {
			THROW_ERROR("Unable to load '%s' from '%s'.", config.c_str(), file);
			return false;
		}
		if (bodies_.count(activators_[name].body_name) == 0) {
			THROW_ERROR("Body '%s' used by activator '%s' not defined.",
				activators_[name].body_name.c_str(), name.c_str());
			return false;
		}

		config = name + ".sprite";
		if (!reader.LookupString(config, &activators_[name].sprite_name)) {
			THROW_ERROR("Unable to load '%s' from '%s'.", config.c_str(), file);
			return false;
		}
		if (sprites_.count(activators_[name].sprite_name) == 0) {
			THROW_ERROR("Sprite '%s' used by activator '%s' not defined.",
				activators_[name].sprite_name.c_str(), name.c_str());
			return false;
		}

		config = name + ".activation_distance";
		if (!reader.LookupFloat(config, &activators_[name].activation_distance)) {
			THROW_ERROR("Unable to load '%s' from '%s'.", config.c_str(), file);
			return false;
		}

		std::string type;
		config = name + ".type";
		if (!reader.LookupString(config, &type)) {
			THROW_ERROR("Unable to load '%s' from '%s'.", config.c_str(), file);
			return false;
		}
		if (type == "door") {
			activators_[name].type = ActivatorConfig::TYPE_DOOR;
		} else {
			THROW_ERROR("Activator type must be 'door'.");
			return false;
		}
	}
	return true;
}

bool Config::LoadCrittersConfig() {
	ConfigReader reader;
	const char* file = "data/critters.cfg";
	if (!reader.Open(file)) {
		return false;
	}
	std::vector<std::string> names;
	reader.GetRootConfigs(&names);
	for (auto name : names) {
		critters_[name].name = name;

		std::string config = name + ".body";
		if (!reader.LookupString(config, &critters_[name].body_name)) {
			THROW_ERROR("Unable to load '%s' from '%s'.", config.c_str(), file);
			return false;
		}
		if (bodies_.count(critters_[name].body_name) == 0) {
			THROW_ERROR("Body '%s' used by critter '%s' not defined.",
				critters_[name].body_name.c_str(), name.c_str());
			return false;
		}

		config = name + ".sprite";
		if (!reader.LookupString(config, &critters_[name].sprite_name)) {
			THROW_ERROR("Unable to load '%s' from '%s'.", config.c_str(), file);
			return false;
		}
		if (sprites_.count(critters_[name].sprite_name) == 0) {
			THROW_ERROR("Sprite '%s' used by critter '%s' not defined.",
				critters_[name].sprite_name.c_str(), name.c_str());
			return false;
		}

		config = name + ".speed";
		if (!reader.LookupFloat(config, &critters_[name].speed)) {
			THROW_ERROR("Unable to load '%s' from '%s'.", config.c_str(), file);
			return false;
		}
		config = name + ".damage";
		if (!reader.LookupInt32(config, &critters_[name].damage)) {
			THROW_ERROR("Unable to load '%s' from '%s'.", config.c_str(), file);
			return false;
		}

		std::string type;
		config = name + ".type";
		if (!reader.LookupString(config, &type)) {
			THROW_ERROR("Unable to load '%s' from '%s'.", config.c_str(), file);
			return false;
		}
		if (type == "zombie") {
			critters_[name].type = CritterConfig::TYPE_ZOMBIE;
		} else {
			THROW_ERROR("Critter type must be 'zombie'.");
			return false;
		}
	}
	return true;
}

bool Config::LoadKitsConfig() {
	ConfigReader reader;
	const char* file = "data/kits.cfg";
	if (!reader.Open(file)) {
		return false;
	}
	std::vector<std::string> names;
	reader.GetRootConfigs(&names);
	for (auto name : names) {
		kits_[name].name = name;

		std::string config = name + ".body";
		if (!reader.LookupString(config, &kits_[name].body_name)) {
			THROW_ERROR("Unable to load '%s' from '%s'.", config.c_str(), file);
			return false;
		}
		if (bodies_.count(kits_[name].body_name) == 0) {
			THROW_ERROR("Body '%s' used by kit '%s' not defined.",
				kits_[name].body_name.c_str(), name.c_str());
			return false;
		}

		config = name + ".sprite";
		if (!reader.LookupString(config, &kits_[name].sprite_name)) {
			THROW_ERROR("Unable to load '%s' from '%s'.", config.c_str(), file);
			return false;
		}
		if (sprites_.count(kits_[name].sprite_name) == 0) {
			THROW_ERROR("Sprite '%s' used by kit '%s' not defined.",
				kits_[name].sprite_name.c_str(), name.c_str());
			return false;
		}

		std::string type;
		config = name + ".type";
		if (!reader.LookupString(config, &type)) {
			THROW_ERROR("Unable to load '%s' from '%s'.", config.c_str(), file);
			return false;
		}
		if (type == "health") {
			kits_[name].type = KitConfig::TYPE_HEALTH;
		} else if (type == "energy") {
			kits_[name].type = KitConfig::TYPE_ENERGY;
		} else if (type == "composite") {
			kits_[name].type = KitConfig::TYPE_COMPOSITE;
		} else {
			THROW_ERROR("Kit type must be 'health', 'energy' or 'composite'.");
			return false;
		}
	}
	return true;
}

bool Config::LoadPlayersConfig() {
	ConfigReader reader;
	const char* file = "data/players.cfg";
	if (!reader.Open(file)) {
		return false;
	}
	std::vector<std::string> names;
	reader.GetRootConfigs(&names);
	for (auto name : names) {
		players_[name].name = name;

		std::string config = name + ".body";
		if (!reader.LookupString(config, &players_[name].body_name)) {
			THROW_ERROR("Unable to load '%s' from '%s'.", config.c_str(), file);
			return false;
		}
		if (bodies_.count(players_[name].body_name) == 0) {
			THROW_ERROR("Body '%s' used by player '%s' not defined.",
				players_[name].body_name.c_str(), name.c_str());
			return false;
		}

		config = name + ".sprite";
		if (!reader.LookupString(config, &players_[name].sprite_name)) {
			THROW_ERROR("Unable to load '%s' from '%s'.", config.c_str(), file);
			return false;
		}
		if (sprites_.count(players_[name].sprite_name) == 0) {
			THROW_ERROR("Sprite '%s' used by player '%s' not defined.",
				players_[name].sprite_name.c_str(), name.c_str());
			return false;
		}

		config = name + ".speed";
		if (!reader.LookupFloat(config, &players_[name].speed)) {
			THROW_ERROR("Unable to load '%s' from '%s'.", config.c_str(), file);
			return false;
		}
		config = name + ".max_health";
		if (!reader.LookupInt32(config, &players_[name].health_max)) {
			THROW_ERROR("Unable to load '%s' from '%s'.", config.c_str(), file);
			return false;
		}
		config = name + ".health_regeneration";
		if (!reader.LookupInt32(config, &players_[name].health_regen)) {
			THROW_ERROR("Unable to load '%s' from '%s'.", config.c_str(), file);
			return false;
		}
		config = name + ".energy_capacity";
		if (!reader.LookupInt32(config, &players_[name].energy_max)) {
			THROW_ERROR("Unable to load '%s' from '%s'.", config.c_str(), file);
			return false;
		}
		config = name + ".energy_regeneration";
		if (!reader.LookupInt32(config, &players_[name].energy_regen)) {
			THROW_ERROR("Unable to load '%s' from '%s'.", config.c_str(), file);
			return false;
		}
	}
	return true;
}

bool Config::LoadProjectilesConfig() {
	ConfigReader reader;
	const char* file = "data/projectiles.cfg";
	if (!reader.Open(file)) {
		return false;
	}
	std::vector<std::string> names;
	reader.GetRootConfigs(&names);
	for (auto name : names) {
		projectiles_[name].name = name;

		std::string config = name + ".body";
		if (!reader.LookupString(config, &projectiles_[name].body_name)) {
			THROW_ERROR("Unable to load '%s' from '%s'.", config.c_str(), file);
			return false;
		}
		if (bodies_.count(projectiles_[name].body_name) == 0) {
			THROW_ERROR("Body '%s' used by projectile '%s' not defined.",
				projectiles_[name].body_name.c_str(), name.c_str());
			return false;
		}

		config = name + ".sprite";
		if (!reader.LookupString(config, &projectiles_[name].sprite_name)) {
			THROW_ERROR("Unable to load '%s' from '%s'.", config.c_str(), file);
			return false;
		}
		if (sprites_.count(projectiles_[name].sprite_name) == 0) {
			THROW_ERROR("Sprite '%s' used by projectile '%s' not defined.",
				projectiles_[name].sprite_name.c_str(), name.c_str());
			return false;
		}

		config = name + ".speed";
		if (!reader.LookupFloat(config, &projectiles_[name].speed)) {
			THROW_ERROR("Unable to load '%s' from '%s'.", config.c_str(), file);
			return false;
		}

		std::string type;
		config = name + ".type";
		if (!reader.LookupString(config, &type)) {
			THROW_ERROR("Unable to load '%s' from '%s'.", config.c_str(), file);
			return false;
		}
		if (type == "rocket") {
			projectiles_[name].type = ProjectileConfig::TYPE_ROCKET;
			config = name + ".explosion_radius";
			if (!reader.LookupFloat(config,
						&projectiles_[name].rocket_config.explosion_radius)) {
				THROW_ERROR("Unable to load '%s' from '%s'.", config.c_str(), file);
				return false;
			}
			config = name + ".explosion_damage";
			if (!reader.LookupInt32(config,
						&projectiles_[name].rocket_config.explosion_damage)) {
				THROW_ERROR("Unable to load '%s' from '%s'.", config.c_str(), file);
				return false;
			}
		} else if (type == "slime") {
			projectiles_[name].type = ProjectileConfig::TYPE_SLIME;
			config = name + ".explosion_radius";
			if (!reader.LookupInt32(config,
						&projectiles_[name].slime_config.explosion_radius)) {
				THROW_ERROR("Unable to load '%s' from '%s'.", config.c_str(), file);
				return false;
			}
		} else {
			THROW_ERROR("Projectile type must be 'rocket' or 'slime'.");
			return false;
		}
	}
	return true;
}

bool Config::LoadWallsConfig() {
	ConfigReader reader;
	const char* file = "data/walls.cfg";
	if (!reader.Open(file)) {
		return false;
	}
	std::vector<std::string> names;
	reader.GetRootConfigs(&names);
	for (auto name : names) {
		walls_[name].name = name;

		std::string config = name + ".body";
		if (!reader.LookupString(config, &walls_[name].body_name)) {
			THROW_ERROR("Unable to load '%s' from '%s'.", config.c_str(), file);
			return false;
		}
		if (bodies_.count(walls_[name].body_name) == 0) {
			THROW_ERROR("Body '%s' used by wall '%s' not defined.",
				walls_[name].body_name.c_str(), name.c_str());
			return false;
		}

		config = name + ".sprite";
		if (!reader.LookupString(config, &walls_[name].sprite_name)) {
			THROW_ERROR("Unable to load '%s' from '%s'.", config.c_str(), file);
			return false;
		}
		if (sprites_.count(walls_[name].sprite_name) == 0) {
			THROW_ERROR("Sprite '%s' used by wall '%s' not defined.",
				walls_[name].sprite_name.c_str(), name.c_str());
			return false;
		}

		std::string type;
		config = name + ".type";
		if (!reader.LookupString(config, &type)) {
			THROW_ERROR("Unable to load '%s' from '%s'.", config.c_str(), file);
			return false;
		}
		if (type == "ordinary") {
			walls_[name].type = WallConfig::TYPE_ORDINARY;
		} else if (type == "unbreakable") {
			walls_[name].type = WallConfig::TYPE_UNBREAKABLE;
		} else if (type == "morphed") {
			walls_[name].type = WallConfig::TYPE_MORPHED;
		} else {
			THROW_ERROR("Wall type must be 'ordinary', 'unbreakable' or 'morphed'.");
			return false;
		}
	}
	return true;
}

bool Config::LoadGunsConfig() {
	ConfigReader reader;
	const char* file = "data/guns.cfg";
	if (!reader.Open(file)) {
		return false;
	}
	std::vector<std::string> names;
	reader.GetRootConfigs(&names);
	for (auto name : names) {
		guns_[name].name = name;

		std::string config = name + ".projectile";
		if (!reader.LookupString(config, &guns_[name].projectile_name)) {
			THROW_ERROR("Unable to load '%s' from '%s'.", config.c_str(), file);
			return false;
		}
		if (projectiles_.count(guns_[name].projectile_name) == 0) {
			THROW_ERROR("Projectile '%s' used by gun '%s' not defined.",
				guns_[name].projectile_name.c_str(), name.c_str());
			return false;
		}

		config = name + ".energy_consumption";
		if (!reader.LookupInt32(config, &guns_[name].energy_consumption)) {
			THROW_ERROR("Unable to load '%s' from '%s'.", config.c_str(), file);
			return false;
		}
	}
	return true;
}

}  // namespace bm
