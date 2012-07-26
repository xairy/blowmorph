#ifndef BLOWMORPH_BASE_ERROR_HPP_
#define BLOWMORPH_BASE_ERROR_HPP_

// WARNING: 'Error' class in NOT thread safe.

// TODO: make it thread safe.

#include <cstdio>
#include <sstream>
#include <cstdarg>
#include <vector>
#include <string>

#include "base/macros.hpp"

namespace bm {

class Error {
public:
  enum ErrorType {
    TYPE_NOTHING,
    TYPE_MEMORY,

    TYPE_CONNECT,
    TYPE_TIME_SYNC,

    TYPE_ENET_INITIALIZE,
    TYPE_ENET_HOST_CREATE,
    TYPE_ENET_SERVICE,
    TYPE_ENET_SET_HOST,
    TYPE_ENET_CONNECT,
    TYPE_ENET_PACKET_CREATE,
    TYPE_ENET_SEND,
    TYPE_ENET_GET_HOST_IP,
    
    TYPE_NEW
  };

  static void Set(ErrorType error) {
    _error = error;
  }
  
  static void Throw(const char* file, unsigned int line, const char* fmt, ...) {
    static char buf[1024];
    
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf) - 1, fmt, args);
    va_end(args);
    
    std::stringstream ss;
    ss << "Error in file " << file << " on line " << line << ":" << std::endl;
    ss << buf << std::endl << '\0';
    
    Error::_error = TYPE_NEW;
    Error::message = ss.str();
  }

  static ErrorType Get() {
    return _error;
  }

  static std::string GetDescription(ErrorType error) {
    switch(error) {
      case TYPE_ENET_INITIALIZE:
        return "Error: unable to initialize enet.";
      case TYPE_ENET_HOST_CREATE:
        return "Error: unable to create enet host.";
      case TYPE_ENET_SERVICE:
        return "Error: unable to service enet host.";
      case TYPE_ENET_SET_HOST:
        return "Error: unable to set host in enet address.";
      case TYPE_ENET_CONNECT:
        return "Error: unable to connect to enet host.";
      case TYPE_TIME_SYNC:
        return "Error: unable to synchronize time.";
      case TYPE_ENET_PACKET_CREATE:
        return "Error: unable to create enet packet.";
      case TYPE_ENET_SEND:
        return "Error: unable to send packet through enet.";
      case TYPE_ENET_GET_HOST_IP:
        return "Error: unable to get enet host ip.";

      case TYPE_CONNECT:
        return "Error: unable to connect.";

      case TYPE_MEMORY:
        return "Error: unable to allocate memory.";
      case TYPE_NEW:
        return Error::message;
      case TYPE_NOTHING:      
      default:
        return "No errors happened.";
    }
  }

  static void Print() {
    fprintf(stderr, "%s\n", GetDescription(Get()).c_str());
  }

private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(Error);

  static ErrorType _error;
  static std::string message;
};

// TODO[24.7.2012 alex]: VA_ARGS?
#define BM_ERROR(msg) bm::Error::Throw(__FILE__, __LINE__, "%s", msg)

} // namespace bm

#endif // BLOWMORPH_BASE_ERROR_HPP_
