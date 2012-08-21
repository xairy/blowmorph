#ifndef BLOWMORPH_SERVER_SETTINGS_MANAGER_HPP_
#define BLOWMORPH_SERVER_SETTINGS_MANAGER_HPP_

#include <map>
#include <string>

namespace bm {

class SettingsManager {
public:
  SettingsManager();
  ~SettingsManager();

  bool Load(const std::string& path);
  //bool Save(const std::string& path);

  std::string GetValue(const std::string& key, std::string def_value);
  int GetValue(const std::string& key, int def_value);
  bool GetValue(const std::string& key, bool def_value);
  float GetValue(const std::string& key, float def_value);
  double GetValue(const std::string& key, double def_value);

private:
  typedef std::map<std::string, std::string> SettingsMap;
  SettingsMap _settings;
};

} // namespace bm

#endif // BLOWMORPH_SERVER_SETTINGS_MANAGER_HPP_
