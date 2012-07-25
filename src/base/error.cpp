#include <strstream>
#include <cstdarg>
#include "base/error.hpp"

bm::Error::ErrorType bm::Error::_error = Error::TYPE_NOTHING;
std::string bm::Error::message = "";

//void Error::Throw(const char* file, unsigned int line, const char* fmt, ...) {
//  static char buf[1024];
//  
//  va_args argp;
//  va_start(argp, fmt);
//  vsnprintf(buf, sizeof(buf) - 1, fmt, argp);
//  va_end(argp);
//  
//  std::strstream ss;
//  ss << "In file " << file << " on line " << line << ":" << std::endl;
//  ss << buf << std::endl;
//  
//  Error::message = ss.str();
//}
//void Error::Chain(const char* file, unsigned int line, const char* fmt, ...) {
//  static char buf[1024];
//  
//  va_args argp;
//  va_start(argp, fmt);
//  vsnprintf(buf, sizeof(buf) - 1, fmt, argp);
//  va_end(argp);
//  
//  std::strstream ss;
//  ss << "In file " << file << " on line " << line << ":" << std::endl;
//  ss << buf << std::endl;
//  
//  Error::message += ss.str();
//}
//void Error::Clear() {
//  Error::message = "";
//}
//
//std::string Error::GetMessage() {
//  return Error::message;
//}