#ifndef BLOWMORPH_ENET_WRAPPER_LIBRARY_HPP_
#define BLOWMORPH_ENET_WRAPPER_LIBRARY_HPP_

#ifdef _MSC_VER
  #ifdef BM_ENET_DLL
    #define BM_ENET_DECL __declspec(dllexport)
  #else
    #define BM_ENET_DECL __declspec(dllimport)
  #endif
#else
  #define BM_ENET_DECL 
#endif

#endif//BLOWMORPH_ENET_WRAPPER_LIBRARY_HPP_