#include "settings_manager.hpp"

#include <string>

#include <libconfig.h>

#include <base/error.hpp>
#include <base/macros.hpp>
#include <base/pstdint.hpp>

#define DEFINE_GET_METHOD(type, lookup_method, get_method) \
type SettingsManager::get_method(const char* key) {        \
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
  config_init(&cfg_);
  if (!config_read_file(&cfg_, path.c_str())) {
    config_destroy(&cfg_);
    BM_ERROR("Unable to read config!");
    return false;
  }
  state_ = STATE_OPENED;
  return true;
}

void SettingsManager::Close() {
  if (state_ == STATE_OPENED) {
    config_destroy(&cfg_);
    state_ = STATE_CLOSED;
  }
}

bool SettingsManager::HasSetting(const char* key) {
  CHECK(state_ == STATE_OPENED);
  return (config_lookup(&cfg_, key) != NULL);
}

// FIXME(xairy): do static_casts.

bool SettingsManager::LookupInt16(const char* key, int16_t* output) {
  CHECK(state_ == STATE_OPENED);
  int tmp;
  int rv = config_lookup_int(&cfg_, key, &tmp);
  if (rv == CONFIG_TRUE) {
    *output = tmp;
    return true;
  }
  return false;
}
DEFINE_GET_METHOD(int16_t, LookupInt16, GetInt16);

bool SettingsManager::LookupUInt16(const char* key, uint16_t* output) {
  CHECK(state_ == STATE_OPENED);
  int tmp;
  int rv = config_lookup_int(&cfg_, key, &tmp);
  if (rv == CONFIG_TRUE) {
    *output = tmp;
    return true;
  }
  return false;
}
DEFINE_GET_METHOD(uint16_t, LookupUInt16, GetUInt16);

bool SettingsManager::LookupInt32(const char* key, int32_t* output) {
  CHECK(state_ == STATE_OPENED);
  int rv = config_lookup_int(&cfg_, key, output);
  return (rv == CONFIG_TRUE);
}
DEFINE_GET_METHOD(int32_t, LookupInt32, GetInt32);

// TODO(xairy): test.
bool SettingsManager::LookupUInt32(const char* key, uint32_t* output) {
  CHECK(state_ == STATE_OPENED);
  int tmp;
  int rv = config_lookup_int(&cfg_, key, &tmp);
  if (rv == CONFIG_TRUE) {
    *output = tmp;
    return true;
  }
  return false;
}
DEFINE_GET_METHOD(uint32_t, LookupUInt32, GetUInt32);

bool SettingsManager::LookupInt64(const char* key, int64_t* output) {
  CHECK(state_ == STATE_OPENED);
  long long tmp;
  int rv = config_lookup_int64(&cfg_, key, &tmp);
  if (rv == CONFIG_TRUE) {
    *output = static_cast<int64_t>(tmp);
    return true;
  }
  return false;
}
DEFINE_GET_METHOD(int64_t, LookupInt64, GetInt64);

// TODO(xairy): test.
bool SettingsManager::LookupUInt64(const char* key, uint64_t* output) {
  CHECK(state_ == STATE_OPENED);
  long long tmp;
  int rv = config_lookup_int64(&cfg_, key, &tmp);
  if (rv == CONFIG_TRUE) {
    *output = static_cast<uint64_t>(tmp);
    return true;
  }
  return false;
}
DEFINE_GET_METHOD(uint64_t, LookupUInt64, GetUInt64);

bool SettingsManager::LookupFloat(const char* key, float* output) {
  CHECK(state_ == STATE_OPENED);
  double tmp;
  int rv = config_lookup_float(&cfg_, key, &tmp);
  if (rv == CONFIG_TRUE) {
    *output = tmp;
    return true;
  }
  return false;
}
DEFINE_GET_METHOD(float, LookupFloat, GetFloat);

bool SettingsManager::LookupDouble(const char* key, double* output) {
  CHECK(state_ == STATE_OPENED);
  int rv = config_lookup_float(&cfg_, key, output);
  return (rv == CONFIG_TRUE);
}
DEFINE_GET_METHOD(double, LookupDouble, GetDouble);

bool SettingsManager::LookupBool(const char* key, bool* output) {
  CHECK(state_ == STATE_OPENED);
  int tmp;
  int rv = config_lookup_bool(&cfg_, key, &tmp);
  if (rv == CONFIG_TRUE) {
    *output = (tmp != 0);
    return true;
  }
  return false;
}
DEFINE_GET_METHOD(bool, LookupBool, GetBool);

bool SettingsManager::LookupString(const char* key, std::string* output) {
  CHECK(state_ == STATE_OPENED);
  const char *tmp;
  int rv = config_lookup_string(&cfg_, key, &tmp);
  if (rv == CONFIG_TRUE) {
    *output = tmp;
    return true;
  }
  return false;
}
DEFINE_GET_METHOD(std::string, LookupString, GetString);

} // namespace bm
