// Copyright (c) 2015 Blowmorph Team

#ifndef BASE_JSON_H_
#define BASE_JSON_H_

#include <string>

#include <jsoncpp/json/json.h>

#include "base/pstdint.h"

namespace bm {

bool ParseFile(const std::string& file,
    Json::Reader* reader, Json::Value* root);

bool GetInt32(const Json::Value& value, int32_t* out);
bool GetUInt32(const Json::Value& value, uint32_t* out);

// bool GetInt64(const Json::Value& value, int64_t* out);
// bool GetUInt64(const Json::Value& value, uint64_t* out);

bool GetFloat32(const Json::Value& value, float32_t* out);
bool GetFloat64(const Json::Value& value, float64_t* out);

bool GetString(const Json::Value& value, std::string* out);
bool GetBool(const Json::Value& value, bool* out);

}  // namespace bm

#endif  // BASE_JSON_H_
