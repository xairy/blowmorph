// Copyright (c) 2015 Blowmorph Team

#ifndef ENGINE_ENGINE_H_
#define ENGINE_ENGINE_H_

#ifdef _MSC_VER
  #ifdef BM_ENGINE_DLL
    #define BM_ENGINE_DECL __declspec(dllexport)
  #else
    #define BM_ENGINE_DECL __declspec(dllimport)
  #endif
#else
  #define BM_ENGINE_DECL
#endif

#endif  // ENGINE_ENGINE_H_
