// Copyright (c) 2013 Blowmorph Team

#include "base/settings_manager.h"

#include <limits>
#include <string>
#include <vector>

#include <libconfig.h>

#include "base/error.h"
#include "base/macros.h"
#include "base/pstdint.h"

#define DEFINE_GET_METHOD(type, lookup_method, get_method) \
type SettingsManager::get_method(const std::string& key) {        \
  type result;                                             \
  bool rv = lookup_method(key, &result);                   \
  CHECK(rv == true);                                       \
  return result;                                           \
}

namespace bm {

SettingsManager::SettingsManager() : state_(STATE_CLOSED) { }
SettingsManager::~SettingsManager() {
  Close();
}

bool SettingsManager::Open(const std::string& path) {
  cfg_ = new config_t();
  CHECK(cfg_ != NULL);
  config_init(cfg_);
  if (!config_read_file(cfg_, path.c_str())) {
    config_destroy(cfg_);
    THROW_ERROR("Unable to read config!");
    return false;
  }
  state_ = STATE_OPENED;
  return true;
}

void SettingsManager::Close() {
  if (state_ == STATE_OPENED) {
    config_destroy(cfg_);
    delete cfg_;
    cfg_ = NULL;
    state_ = STATE_CLOSED;
  }
}

bool SettingsManager::HasSetting(const std::string& key) {
  CHECK(state_ == STATE_OPENED);
  return (config_lookup(cfg_, key.c_str()) != NULL);
}

bool SettingsManager::LookupInt16(const std::string& key, int16_t* output) {
  CHECK(state_ == STATE_OPENED);
  long long tmp;  // NOLINT
  int rv = config_lookup_int64(cfg_, key.c_str(), &tmp);
  if (rv != CONFIG_TRUE) {
    return false;
  }
  if (tmp < std::numeric_limits<int16_t>::min() ||
      tmp > std::numeric_limits<int16_t>::max()) {
    return false;
  }
  *output = static_cast<int16_t>(tmp);
  return true;
}
DEFINE_GET_METHOD(int16_t, LookupInt16, GetInt16);

bool SettingsManager::LookupUInt16(const std::string& key, uint16_t* output) {
  CHECK(state_ == STATE_OPENED);
  long long tmp;  // NOLINT
  int rv = config_lookup_int64(cfg_, key.c_str(), &tmp);
  if (rv != CONFIG_TRUE) {
    return false;
  }
  if (tmp < 0 || tmp > std::numeric_limits<uint16_t>::max()) {
    return false;
  }
  *output = static_cast<uint16_t>(tmp);
  return true;
}
DEFINE_GET_METHOD(uint16_t, LookupUInt16, GetUInt16);

bool SettingsManager::LookupInt32(const std::string& key, int32_t* output) {
  CHECK(state_ == STATE_OPENED);
  long long tmp;  // NOLINT
  int rv = config_lookup_int64(cfg_, key.c_str(), &tmp);
  if (rv != CONFIG_TRUE) {
    return false;
  }
  if (tmp < std::numeric_limits<int32_t>::min() ||
      tmp > std::numeric_limits<int32_t>::max()) {
    return false;
  }
  *output = static_cast<int32_t>(tmp);
  return true;
}
DEFINE_GET_METHOD(int32_t, LookupInt32, GetInt32);

bool SettingsManager::LookupUInt32(const std::string& key, uint32_t* output) {
  CHECK(state_ == STATE_OPENED);
  long long tmp;  // NOLINT
  int rv = config_lookup_int64(cfg_, key.c_str(), &tmp);
  if (rv != CONFIG_TRUE) {
    return false;
  }
  if (tmp < 0 || tmp > std::numeric_limits<uint32_t>::max()) {
    return false;
  }
  *output = static_cast<uint32_t>(tmp);
  return true;
}
DEFINE_GET_METHOD(uint32_t, LookupUInt32, GetUInt32);

bool SettingsManager::LookupInt64(const std::string& key, int64_t* output) {
  CHECK(state_ == STATE_OPENED);
  long long tmp;  // NOLINT
  int rv = config_lookup_int64(cfg_, key.c_str(), &tmp);
  if (rv != CONFIG_TRUE) {
    return false;
  }
  if (tmp < std::numeric_limits<int64_t>::min() ||
      tmp > std::numeric_limits<int64_t>::max()) {
    return false;
  }
  *output = static_cast<int64_t>(tmp);
  return true;
}
DEFINE_GET_METHOD(int64_t, LookupInt64, GetInt64);

bool SettingsManager::LookupUInt64(const std::string& key, uint64_t* output) {
  CHECK(state_ == STATE_OPENED);
  long long tmp;  // NOLINT
  int rv = config_lookup_int64(cfg_, key.c_str(), &tmp);
  if (rv != CONFIG_TRUE) {
    return false;
  }
  if (tmp < 0) {
    return false;
  }
  *output = static_cast<uint64_t>(tmp);
  return true;
}
DEFINE_GET_METHOD(uint64_t, LookupUInt64, GetUInt64);

bool SettingsManager::LookupFloat(const std::string& key, float* output) {
  CHECK(state_ == STATE_OPENED);
  double tmp;
  int rv = config_lookup_float(cfg_, key.c_str(), &tmp);
  if (rv == CONFIG_TRUE) {
    *output = static_cast<float>(tmp);
    return true;
  }
  return false;
}
DEFINE_GET_METHOD(float, LookupFloat, GetFloat);

bool SettingsManager::LookupDouble(const std::string& key, double* output) {
  CHECK(state_ == STATE_OPENED);
  int rv = config_lookup_float(cfg_, key.c_str(), output);
  return (rv == CONFIG_TRUE);
}
DEFINE_GET_METHOD(double, LookupDouble, GetDouble);

bool SettingsManager::LookupBool(const std::string& key, bool* output) {
  CHECK(state_ == STATE_OPENED);
  int tmp;
  int rv = config_lookup_bool(cfg_, key.c_str(), &tmp);
  if (rv == CONFIG_TRUE) {
    *output = (tmp != 0);
    return true;
  }
  return false;
}
DEFINE_GET_METHOD(bool, LookupBool, GetBool);

bool SettingsManager::LookupString(const std::string& key,
    std::string* output) {
  CHECK(state_ == STATE_OPENED);
  const char *tmp;
  int rv = config_lookup_string(cfg_, key.c_str(), &tmp);
  if (rv == CONFIG_TRUE) {
    *output = tmp;
    return true;
  }
  return false;
}
DEFINE_GET_METHOD(std::string, LookupString, GetString);

bool SettingsManager::LookupInt32List(const std::string& key,
    std::vector<int32_t>* output) {
  CHECK(state_ == STATE_OPENED);
  config_setting_t* setting = config_lookup(cfg_, key.c_str());
  if (setting == NULL) {
    return false;
  }
  if (config_setting_is_array(setting) == CONFIG_FALSE) {
    return false;
  }
  int length = config_setting_length(setting);
  if (length == 0) {
    return false;
  }
  output->clear();
  for (int i = 0; i < length; i++) {
    config_setting_t* value_setting = config_setting_get_elem(setting, i);
    if (value_setting == NULL) {
      return false;
    }
    int value_type = config_setting_type(value_setting);
    if (value_type != CONFIG_TYPE_INT && value_type != CONFIG_TYPE_INT64) {
      return false;
    }
    long long value = config_setting_get_int64(value_setting);  // NOLINT
    if (value < std::numeric_limits<int32_t>::min() ||
        value > std::numeric_limits<int32_t>::max()) {
      return false;
    }
    output->push_back(static_cast<int32_t>(value));
  }
  return true;
}

#undef DEFINE_GET_METHOD

}  // namespace bm
