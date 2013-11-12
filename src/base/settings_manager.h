// Copyright (c) 2013 Blowmorph Team

#ifndef SRC_BASE_SETTINGS_MANAGER_H_
#define SRC_BASE_SETTINGS_MANAGER_H_

#include <string>

#include <libconfig.h>

#include "base/pstdint.h"

namespace bm {

class SettingsManager {
 public:
  SettingsManager();
  ~SettingsManager();

  bool Open(const std::string& path);
  void Close();

  bool HasSetting(const char* key);

  bool LookupInt16(const char* key, int16_t* output);
  bool LookupUInt16(const char* key, uint16_t* output);
  int16_t GetInt16(const char* key);
  uint16_t GetUInt16(const char* key);

  bool LookupInt32(const char* key, int32_t* output);
  bool LookupUInt32(const char* key, uint32_t* output);
  int32_t GetInt32(const char* key);
  uint32_t GetUInt32(const char* key);

  bool LookupInt64(const char* key, int64_t* output);
  bool LookupUInt64(const char* key, uint64_t* output);
  int64_t GetInt64(const char* key);
  uint64_t GetUInt64(const char* key);

  bool LookupFloat(const char* key, float* output);
  bool LookupDouble(const char* key, double* output);
  float GetFloat(const char* key);
  double GetDouble(const char* key);

  bool LookupBool(const char* key, bool* output);
  bool LookupString(const char* key, std::string* output);
  bool GetBool(const char* key);
  std::string GetString(const char* key);

 private:
  enum {
    STATE_CLOSED,
    STATE_OPENED
  } state_;

  config_t cfg_;
};

}  // namespace bm

#endif  // SRC_BASE_SETTINGS_MANAGER_H_
