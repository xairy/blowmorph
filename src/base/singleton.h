// Copyright (c) 2015 Blowmorph Team

#ifndef BASE_SINGLETON_H_
#define BASE_SINGLETON_H_

#include "base/macros.h"

// WARNING: Singleton is NOT thread safe.

template <typename T>
class Singleton {
 public:
  static T* GetInstance() {
    static T _instance;
    return &_instance;
  }

 protected:
 	Singleton() { }
  DISALLOW_COPY_AND_ASSIGN(Singleton);
};

#endif  // BASE_SINGLETON_H_
