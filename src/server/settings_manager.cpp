#include "settings_manager.hpp"

#include <cstdlib>
#include <cstring>

#include <map>
#include <string>

#include <pugixml.hpp>

#include <base/error.hpp>

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

  for(pugi::xml_node x = document.first_child(); x; x = x.next_sibling()) {
    for(pugi::xml_node y = x.first_child(); y; y = y.next_sibling()) {
      for(pugi::xml_node z = y.first_child(); z; z = z.next_sibling()) {
        if(z.type() == pugi::node_pcdata) {
          std::string key = std::string(x.name()) + "." + std::string(y.name());
          std::string value = std::string(z.value());
          _settings[key] = value;
          printf("'%s' '%s'\n", key.c_str(), value.c_str());
        } else if(z.type() == pugi::node_element) {
          std::string key = std::string(x.name()) + "." + std::string(y.name()) + "." + std::string(z.name());
          std::string value = std::string(z.child_value());
          _settings[key] = value;
          printf("'%s' '%s'\n", key.c_str(), value.c_str());
        }
      }
    }
  }

  return true;
}

std::string SettingsManager::GetValue(const std::string& key, std::string def_value) const {
  SettingsMap::const_iterator i = _settings.find(key);
  if(i != _settings.end()) {
    return i->second;
  }
  printf("Warning: returning default value.\n");
  return def_value;
}
int SettingsManager::GetValue(const std::string& key, int def_value) const {
  SettingsMap::const_iterator i = _settings.find(key);
  if(i != _settings.end()) {
    // XXX[21.08.2012 xairy]: malformed int?
    return atoi(i->second.c_str());
  }
  printf("Warning: returning default value.\n");
  return def_value;
}
bool SettingsManager::GetValue(const std::string& key, bool def_value) const {
  SettingsMap::const_iterator i = _settings.find(key);
  if(i != _settings.end()) {
    std::string value = i->second;
    return (value == "1" || value == "yes" || value == "true");
  }
  printf("Warning: returning default value.\n");
  return def_value;
}
float SettingsManager::GetValue(const std::string& key, float def_value) const {
  SettingsMap::const_iterator i = _settings.find(key);
  if(i != _settings.end()) {
    // XXX[21.08.2012 xairy]: malformed float?
    return static_cast<float>(atof(i->second.c_str()));
  }
  printf("Warning: returning default value.\n");
  return def_value;
}
double SettingsManager::GetValue(const std::string& key, double def_value) const {
  SettingsMap::const_iterator i = _settings.find(key);
  if(i != _settings.end()) {
    // XXX[21.08.2012 xairy]: malformed double?
    return atof(i->second.c_str());
  }
  printf("Warning: returning default value.\n");
  return def_value;
}

} // namespace bm