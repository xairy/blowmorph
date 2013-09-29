// Copyright (c) 2013 Blowmorph Team

#ifndef SRC_BASE_SINGLETON_H_
#define SRC_BASE_SINGLETON_H_

// WARNING: Singleton is NOT thread safe.

template <typename T>
class Singleton {
 public:
  static T* GetInstance() {
    static T _instance;
    return &_instance;
  }

 private:
  // Forbid creating Singleton itself.
  Singleton();
  ~Singleton();
};

#endif  // SRC_BASE_SINGLETON_H_
