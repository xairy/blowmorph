// Copyright (c) 2013 Blowmorph Team

#ifndef BASE_SETTINGS_MANAGER_H_
#define BASE_SETTINGS_MANAGER_H_

#include <string>
#include <vector>

#include <libconfig.h>

#include "base/pstdint.h"

namespace bm {

class SettingsManager {
 public:
  SettingsManager();
  ~SettingsManager();

  bool Open(const std::string& path);
  void Close();

  bool HasSetting(const std::string& key);

  bool LookupInt16(const std::string& key, int16_t* output);
  bool LookupUInt16(const std::string& key, uint16_t* output);
  int16_t GetInt16(const std::string& key);
  uint16_t GetUInt16(const std::string& key);

  bool LookupInt32(const std::string& key, int32_t* output);
  bool LookupUInt32(const std::string& key, uint32_t* output);
  int32_t GetInt32(const std::string& key);
  uint32_t GetUInt32(const std::string& key);

  bool LookupInt64(const std::string& key, int64_t* output);
  bool LookupUInt64(const std::string& key, uint64_t* output);
  int64_t GetInt64(const std::string& key);
  uint64_t GetUInt64(const std::string& key);

  bool LookupFloat(const std::string& key, float* output);
  bool LookupDouble(const std::string& key, double* output);
  float GetFloat(const std::string& key);
  double GetDouble(const std::string& key);

  bool LookupBool(const std::string& key, bool* output);
  bool LookupString(const std::string& key, std::string* output);
  bool GetBool(const std::string& key);
  std::string GetString(const std::string& key);

  bool LookupInt32List(const std::string& key, std::vector<int32_t>* output);

 private:
  enum {
    STATE_CLOSED,
    STATE_OPENED
  } state_;

  config_t cfg_;
};

}  // namespace bm

#endif  // BASE_SETTINGS_MANAGER_H_
