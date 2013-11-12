// Copyright (c) 2013 Blowmorph Team

#ifndef BASE_SINGLETON_H_
#define BASE_SINGLETON_H_

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

#endif  // BASE_SINGLETON_H_
