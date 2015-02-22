// Copyright (c) 2015 Blowmorph Team

#include "base/config_reader.h"

#include <limits>
#include <string>
#include <vector>

#include <libconfig.h>

#include "base/error.h"
#include "base/macros.h"
#include "base/pstdint.h"

#define DEFINE_GET_METHOD(type, lookup_method, get_method) \
type ConfigReader::get_method(const std::string& key) { \
  type result;                                             \
  bool rv = lookup_method(key, &result);                   \
  CHECK(rv == true);                                       \
  return result;                                           \
}

namespace bm {

ConfigReader::ConfigReader() : state_(STATE_CLOSED) { }
ConfigReader::~ConfigReader() {
  Close();
}

bool ConfigReader::Open(const std::string& path) {
  cfg_ = new config_t();
  CHECK(cfg_ != NULL);
  config_init(cfg_);
  if (!config_read_file(cfg_, path.c_str())) {
    config_destroy(cfg_);
    THROW_ERROR("Unable to read config \"%s\"!", path.c_str());
    return false;
  }
  state_ = STATE_OPENED;
  return true;
}

void ConfigReader::Close() {
  if (state_ == STATE_OPENED) {
    config_destroy(cfg_);
    delete cfg_;
    cfg_ = NULL;
    state_ = STATE_CLOSED;
  }
}

bool ConfigReader::HasSetting(const std::string& key) {
  CHECK(state_ == STATE_OPENED);
  return (config_lookup(cfg_, key.c_str()) != NULL);
}

bool ConfigReader::LookupInt16(const std::string& key, int16_t* output) {
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

bool ConfigReader::LookupUInt16(const std::string& key, uint16_t* output) {
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

bool ConfigReader::LookupInt32(const std::string& key, int32_t* output) {
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

bool ConfigReader::LookupUInt32(const std::string& key, uint32_t* output) {
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

bool ConfigReader::LookupInt64(const std::string& key, int64_t* output) {
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

bool ConfigReader::LookupUInt64(const std::string& key, uint64_t* output) {
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

bool ConfigReader::LookupFloat(const std::string& key, float* output) {
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

bool ConfigReader::LookupDouble(const std::string& key, double* output) {
  CHECK(state_ == STATE_OPENED);
  int rv = config_lookup_float(cfg_, key.c_str(), output);
  return (rv == CONFIG_TRUE);
}
DEFINE_GET_METHOD(double, LookupDouble, GetDouble);

bool ConfigReader::LookupBool(const std::string& key, bool* output) {
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

bool ConfigReader::LookupString(const std::string& key,
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

bool ConfigReader::LookupInt32List(const std::string& key,
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

void ConfigReader::GetRootConfigs(std::vector<std::string>* output) {
  CHECK(state_ == STATE_OPENED);
  output->clear();
  config_setting_t* root_setting = config_root_setting(cfg_);
  int length = config_setting_length(root_setting);
  for (int i = 0; i < length; i++) {
    config_setting_t* setting = config_setting_get_elem(root_setting, i);
    CHECK(setting != NULL);
    const char* name = config_setting_name(setting);
    CHECK(name != NULL);
    output->push_back(std::string(name));
  }
}

#undef DEFINE_GET_METHOD

}  // namespace bm
