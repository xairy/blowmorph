// Copyright (c) 2015 Andrey Konovalov

#ifndef NET_DLL_H_
#define NET_DLL_H_

#ifdef _MSC_VER
  #ifdef BM_NET_DLL
    #define BM_NET_DECL __declspec(dllexport)
  #else
    #define BM_NET_DECL __declspec(dllimport)
  #endif
#else
  #define BM_NET_DECL
#endif

#endif  // NET_DLL_H_
