// Copyright (c) 2015 Blowmorph Team

#ifndef BASE_ERROR_H_
#define BASE_ERROR_H_

// WARNING: 'Error' class in NOT thread safe.
// TODO(xairy): make it thread safe.

#include <cstdarg>
#include <cstdio>

#include <vector>
#include <sstream>
#include <string>

#include "base/base.h"
#include "base/macros.h"

namespace bm {

class Error {
 public:
  static void Print() {
    if (messages.empty()) {
      fprintf(stderr, "No errors have been reported.\n");
      return;
    }
    std::vector<std::string>::const_iterator itr;
    for (itr = messages.begin(); itr != messages.end(); ++itr) {
      fprintf(stderr, "%s\n", itr->c_str());
    }
  }

  static void Report(const char* file, unsigned int line,
                     const char* fmt, ...) {
    static char buf[1024];

    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf) - 1, fmt, args);
    va_end(args);

    std::stringstream ss;
    ss << "Error: " << buf << " (" << file << ":" << line << ")";

    Error::messages.push_back(ss.str());
  }

 private:
  BM_BASE_DECL static std::vector<std::string> messages;

  DISALLOW_IMPLICIT_CONSTRUCTORS(Error);
};

#define REPORT_ERROR(msg, ...) \
  bm::Error::Report(__FILE__, __LINE__, msg, ##__VA_ARGS__)

#define REPORT_WARNING(msg, ...) \
  fprintf(stderr, "Warning: " msg "\n", ##__VA_ARGS__)

}  // namespace bm

#endif  // BASE_ERROR_H_
