// Copyright (c) 2015 Blowmorph Team

#ifndef BASE_JSON_H_
#define BASE_JSON_H_

#include <string>

#include <jsoncpp/json/json.h>

#include "base/dll.h"
#include "base/pstdint.h"

namespace bm {

BM_BASE_DECL bool ParseFile(const std::string& file,
    Json::Reader* reader, Json::Value* root);

BM_BASE_DECL bool GetInt32(const Json::Value& value, int32_t* out);
BM_BASE_DECL bool GetUInt32(const Json::Value& value, uint32_t* out);

// BM_BASE_DECL bool GetInt64(const Json::Value& value, int64_t* out);
// BM_BASE_DECL bool GetUInt64(const Json::Value& value, uint64_t* out);

BM_BASE_DECL bool GetFloat32(const Json::Value& value, float32_t* out);
BM_BASE_DECL bool GetFloat64(const Json::Value& value, float64_t* out);

BM_BASE_DECL bool GetString(const Json::Value& value, std::string* out);
BM_BASE_DECL bool GetBool(const Json::Value& value, bool* out);

}  // namespace bm

#endif  // BASE_JSON_H_
