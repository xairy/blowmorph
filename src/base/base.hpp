#ifndef BM_BASE_BASE_HPP_
#define BM_BASE_BASE_HPP_

#ifdef _MSC_VER
  #ifdef BM_BASE_DLL
    #define BM_BASE_DECL __declspec(dllexport)
  #else
    #define BM_BASE_DECL __declspec(dllimport)
  #endif
#else
  #define BM_BASE_DECL 
#endif

#endif // BM_BASE_BASE_HPP_
