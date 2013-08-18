#ifndef BLOWMORPH_BASE_SETTINGS_MANAGER_HPP_
#define BLOWMORPH_BASE_SETTINGS_MANAGER_HPP_

#include <map>
#include <string>

namespace bm {

class SettingsManager {
public:
  SettingsManager();
  ~SettingsManager();

  bool Load(const std::string& path);
  //bool Save(const std::string& path);

  std::string GetValue(const std::string& key, std::string def_value) const;
  int GetValue(const std::string& key, int def_value) const;
  bool GetValue(const std::string& key, bool def_value) const;
  float GetValue(const std::string& key, float def_value) const;
  double GetValue(const std::string& key, double def_value) const;

private:
  typedef std::map<std::string, std::string> SettingsMap;
  SettingsMap _settings;
};

} // namespace bm

#endif // BLOWMORPH_BASE_SETTINGS_MANAGER_HPP_
