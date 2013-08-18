#include "settings_manager.hpp"

#include <cstdlib>
#include <cstring>

#include <map>
#include <string>

#include <pugixml.hpp>

#include <base/error.hpp>
#include <base/macros.hpp>

namespace {

std::string BuildKey(const pugi::xml_node& node) {
  CHECK(node.type() == pugi::node_element);
  if(node.parent().type() == pugi::node_document) {
    return std::string(node.name());
  } else if(node.parent().type() == pugi::node_element) {
    return BuildKey(node.parent()) + "." + std::string(node.name());
  }
  CHECK(false);
  return "";
}

struct SettingsWalker : public pugi::xml_tree_walker {
  typedef std::map<std::string, std::string> SettingsMap;

  SettingsWalker(SettingsMap* settings)
    : pugi::xml_tree_walker(), _settings(settings) { }

  virtual bool for_each(pugi::xml_node& node) {
    if(node.type() == pugi::node_pcdata) {
      std::string key = BuildKey(node.parent());
      std::string value = std::string(node.value());
      (*_settings)[key] = value;
      printf("'%s' = '%s'\n", key.c_str(), value.c_str());
    }
    return true;
  }

  SettingsMap* _settings;
};

} // anonymous namespace

namespace bm {

SettingsManager::SettingsManager() { }
SettingsManager::~SettingsManager() { }

bool SettingsManager::Load(const std::string& path) {
  pugi::xml_document document;
  pugi::xml_parse_result parse_result = document.load_file(path.c_str());
  if(!parse_result) {
    BM_ERROR("Unable to parse settings.");
    return false;
  }

  SettingsWalker walker(&_settings);
  document.traverse(walker);

  return true;
}

std::string SettingsManager::GetValue(const std::string& key, std::string def_value) const {
  SettingsMap::const_iterator i = _settings.find(key);
  if(i != _settings.end()) {
    return i->second;
  }
  printf("Warning: returning default value for '%s'.\n", key.c_str());
  return def_value;
}
int SettingsManager::GetValue(const std::string& key, int def_value) const {
  SettingsMap::const_iterator i = _settings.find(key);
  if(i != _settings.end()) {
    // XXX[21.08.2012 xairy]: malformed int?
    return atoi(i->second.c_str());
  }
  printf("Warning: returning default value for '%s'.\n", key.c_str());
  return def_value;
}
bool SettingsManager::GetValue(const std::string& key, bool def_value) const {
  SettingsMap::const_iterator i = _settings.find(key);
  if(i != _settings.end()) {
    std::string value = i->second;
    return (value == "1" || value == "yes" || value == "true");
  }
  printf("Warning: returning default value for '%s'.\n", key.c_str());
  return def_value;
}
float SettingsManager::GetValue(const std::string& key, float def_value) const {
  SettingsMap::const_iterator i = _settings.find(key);
  if(i != _settings.end()) {
    // XXX[21.08.2012 xairy]: malformed float?
    return static_cast<float>(atof(i->second.c_str()));
  }
  printf("Warning: returning default value for '%s'.\n", key.c_str());
  return def_value;
}
double SettingsManager::GetValue(const std::string& key, double def_value) const {
  SettingsMap::const_iterator i = _settings.find(key);
  if(i != _settings.end()) {
    // XXX[21.08.2012 xairy]: malformed double?
    return atof(i->second.c_str());
  }
  printf("Warning: returning default value for '%s'.\n", key.c_str());
  return def_value;
}

} // namespace bm
