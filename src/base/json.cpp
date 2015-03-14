// Copyright (c) 2015 Blowmorph Team

#include "base/json.h"

#include <string>

#include <jsoncpp/json/json.h>

#include "base/pstdint.h"

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