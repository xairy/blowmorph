// Copyright (c) 2015 Blowmorph Team

#ifndef BASE_CONFIG_READER_H_
#define BASE_CONFIG_READER_H_

#include <string>
#include <vector>

#include "base/base.h"
#include "base/pstdint.h"

struct config_t;

namespace bm {

// FIXME: make methods const.
class ConfigReader {
 public:
  BM_BASE_DECL ConfigReader();
  BM_BASE_DECL ~ConfigReader();

  BM_BASE_DECL bool Open(const std::string& path);
  BM_BASE_DECL void Close();

  BM_BASE_DECL bool HasSetting(const std::string& key);

  BM_BASE_DECL bool LookupInt16(const std::string& key, int16_t* output);
  BM_BASE_DECL bool LookupUInt16(const std::string& key, uint16_t* output);
  BM_BASE_DECL int16_t GetInt16(const std::string& key);
  BM_BASE_DECL uint16_t GetUInt16(const std::string& key);

  BM_BASE_DECL bool LookupInt32(const std::string& key, int32_t* output);
  BM_BASE_DECL bool LookupUInt32(const std::string& key, uint32_t* output);
  BM_BASE_DECL int32_t GetInt32(const std::string& key);
  BM_BASE_DECL uint32_t GetUInt32(const std::string& key);

  BM_BASE_DECL bool LookupInt64(const std::string& key, int64_t* output);
  BM_BASE_DECL bool LookupUInt64(const std::string& key, uint64_t* output);
  BM_BASE_DECL int64_t GetInt64(const std::string& key);
  BM_BASE_DECL uint64_t GetUInt64(const std::string& key);

  BM_BASE_DECL bool LookupFloat(const std::string& key, float* output);
  BM_BASE_DECL bool LookupDouble(const std::string& key, double* output);
  BM_BASE_DECL float GetFloat(const std::string& key);
  BM_BASE_DECL double GetDouble(const std::string& key);

  BM_BASE_DECL bool LookupBool(const std::string& key, bool* output);
  BM_BASE_DECL bool LookupString(const std::string& key, std::string* output);
  BM_BASE_DECL bool GetBool(const std::string& key);
  BM_BASE_DECL std::string GetString(const std::string& key);

  BM_BASE_DECL bool LookupInt32List(const std::string& key,
    std::vector<int32_t>* output);

  BM_BASE_DECL void GetRootConfigs(std::vector<std::string>* output);

 private:
  enum {
    STATE_CLOSED,
    STATE_OPENED
  } state_;

  config_t* cfg_;
};

}  // namespace bm

#endif  // BASE_CONFIG_READER_H_
