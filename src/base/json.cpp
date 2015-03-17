// Copyright (c) 2015 Blowmorph Team

#include "base/json.h"

#include <fstream>  // NOLINT
#include <string>

#include <jsoncpp/json/json.h>

#include "base/pstdint.h"
#include "base/error.h"

namespace bm {

bool ParseFile(const std::string& file,
      Json::Reader* reader, Json::Value* root) {
  std::fstream stream(file);
  if (!stream.is_open()) {
    REPORT_ERROR("Can't open file '%s'.", file.c_str());
    return false;
  }

  bool success = reader->parse(stream, *root, false);
  if (!success) {
      std::string error = reader->getFormatedErrorMessages();
      REPORT_ERROR("Can't parse '%s':\n%s", file.c_str(), error.c_str());
      return false;
  }

  return true;
}

bool GetInt32(const Json::Value& value, int32_t* out) {
  if (value == Json::Value::null) {
    return false;
  }
  if (!value.isInt()) {
    return false;
  }
  *out = value.asInt();
  return true;
}

bool GetUInt32(const Json::Value& value, uint32_t* out) {
  if (value == Json::Value::null) {
    return false;
  }
  if (!value.isUInt()) {
    return false;
  }
  *out = value.asUInt();
  return true;
}

bool GetFloat32(const Json::Value& value, float32_t* out) {
  if (value == Json::Value::null) {
    return false;
  }
  if (!value.isDouble()) {
    return false;
  }
  *out = value.asFloat();
  return true;
}

bool GetFloat64(const Json::Value& value, float64_t* out) {
  if (value == Json::Value::null) {
    return false;
  }
  if (!value.isDouble()) {
    return false;
  }
  *out = value.asDouble();
  return true;
}

bool GetString(const Json::Value& value, std::string* out) {
  if (value == Json::Value::null) {
    return false;
  }
  if (!value.isString()) {
    return false;
  }
  *out = value.asString();
  return true;
}

bool GetBool(const Json::Value& value, bool* out) {
  if (value == Json::Value::null) {
    return false;
  }
  if (!value.isBool()) {
    return false;
  }
  *out = value.asBool();
  return true;
}

}  // namespace bm
