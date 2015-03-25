// Copyright (c) 2015 Blowmorph Team

#ifndef BASE_DLL_H_
#define BASE_DLL_H_

#ifdef _MSC_VER
  #ifdef BM_BASE_DLL
    #define BM_BASE_DECL __declspec(dllexport)
  #else
    #define BM_BASE_DECL __declspec(dllimport)
  #endif
#else
  #define BM_BASE_DECL
#endif

#endif  // BASE_DLL_H_
