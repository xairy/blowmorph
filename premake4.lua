function copy(src, dst)
  local action = "python \"" ..  path.join(os.getcwd(), "copy-data.py")  .. "\""
  src = "\"" .. src .. "\""
  dst = "\"" .. dst .. "\""
  cwd = "\"" .. os.getcwd() .. "\""
  postbuildcommands { action .. " " .. cwd .. " " .. src .. " " .. dst }
end

function resource(src, dst, proj)
  copy(src, path.join("build", dst))
  if proj == nil then
    copy(src, path.join("bin", dst))
  else
    copy(src, path.join(path.join("bin", proj), dst))
  end
end

function windows_libdir(basePath)
  for _,arch in pairs({"x32", "x64"}) do
    for _,conf in pairs({"debug", "release"}) do
      for _, plat in pairs({"vs2008"}) do
        local confpath = plat .. "/" .. arch .. "/" .. conf
        configuration { arch, conf, plat }
          libdirs { path.join(basePath, confpath) }
      end
    end
  end
  
  configuration "*"
end

function windows_binary(basePath, dllName, proj)
  for _,arch in pairs({"x32", "x64"}) do
    for _,conf in pairs({"debug", "release"}) do
      for _, plat in pairs({"vs2008"}) do
        local confpath = plat .. "/" .. arch .. "/" .. conf
        configuration { arch, conf, plat }
          resource(path.join(path.join(basePath, confpath), dllName), dllName, proj)
      end
    end
  end
  
  configuration "*"
end

newaction {
  trigger = 'clean',
  description = 'Cleans up the project.',
  shortname = "clean",
  
  execute = function()
    os.rmdir("bin")
    os.rmdir("build")
  end
}

solution "blowmorph"
  configurations { "debug", "release" }
  platforms { "x32", "x64" }

  location "build"
  targetdir "bin"
  
  flags { "FatalWarnings", "NoRTTI", "NoExceptions" }
  
  configuration { "windows" }
    defines { "WIN32", "_WIN32" }
    defines { "_CRT_SECURE_NO_WARNINGS", "_CRT_NONSTDC_NO_DEPRECATE" }
    
  configuration { "debug" }
    defines { "DEBUG" }
    flags { "Symbols" }

  configuration { "release" }
    defines { "NDEBUG" }
    flags { "Optimize" }
  
  project "client"
    kind "ConsoleApp"
    language "C++"
    
    includedirs { "include/", "src/" }
    files { "src/client/**.cpp",
            "src/client/**.hpp" }
    
    links { "bm-base" }
    links { "bm-enet" }
    links { "interpolator" }
    
    --temporary hack
    includedirs { "ext-libs/enet/include" }  
    
    configuration "linux"
      includedirs {
        "/usr/include/freetype2",
        "ext-libs/glm/include"
      }
      links {
        "SDLmain",
        "SDL", 
        "freeimage",
        "GLEW",
        "freetype"
      }
    
    configuration "windows"
      includedirs { 
        "ext-libs/SDL1.2/include",
        "ext-libs/glew/include",
        "ext-libs/glm/include",
        "ext-libs/FreeImage/include",
        "ext-libs/freetype2/include"
      }
      links {
        "SDLmain",
        "SDL",
        "freeimage",
        "opengl32",
        "glu32",
        "glew32",
        "freetype",
        "ws2_32",
        "winmm"
      }
    
    windows_libdir("ext-libs/SDL1.2/bin")
    windows_libdir("ext-libs/glew/bin")
    windows_libdir("ext-libs/enet/bin")
    windows_libdir("ext-libs/FreeImage/bin")
    windows_libdir("ext-libs/freetype2/bin")
    
    windows_binary("ext-libs/SDL1.2/bin", "SDL.dll")
    windows_binary("ext-libs/glew/bin", "glew32.dll")
    windows_binary("ext-libs/FreeImage/bin", "FreeImage.dll")
    windows_binary("ext-libs/freetype2/bin", "freetype.dll")
    
    resource("data", "data")
  
  project "bm-base"
    kind "SharedLib"
    language "C++"
    
    defines { "BM_BASE_DLL" }
    includedirs { "include/", "src/" }
    files { "src/base/**.cpp",
            "src/base/**.hpp" }

  project "bm-enet"
    kind "SharedLib"
    language "C++"
      
    defines { "BM_ENET_DLL" }
    includedirs { "include/", "src/" }
    files { "src/enet-wrapper/**.cpp",
            "src/enet-wrapper/**.hpp" }
     
    links { "bm-base" }
        
    includedirs { "ext-libs/enet/include" }      
    links { "enet" }
    for _,arch in pairs({"x32", "x64"}) do
      for _,conf in pairs({"debug", "release"}) do
        local confpath = arch .. "/" .. conf
        configuration { arch, conf, "vs2008" }
          libdirs { path.join("ext-libs/enet/bin/vs2008", confpath) }
      end
    end
    
    configuration "windows"
      links {
        "ws2_32",
        "winmm"
      }
  
  project "interpolator"
    kind "StaticLib"
    language "C++"
                  
    includedirs { "include/", "src/" }
    
    files { "src/interpolator/**.cpp",
            "src/interpolator/**.hpp" }
      
  project "server"
    kind "ConsoleApp"
    language "C++"
    
    includedirs { "include/", "src/" }
    files { "src/server/**.cpp",
            "src/server/**.hpp" }
    
    links { "bm-base" }
    links { "bm-enet" }
    
    --temporary hack
    includedirs { "ext-libs/enet/include" }
    
    resource("data", "data")
  
  project "sample-server"
    kind "ConsoleApp"
    language "C++"
    
    includedirs { "include/", "src/" }
    files { "src/enet-sample/server.cpp" }
    
    links { "bm-base" }
    links { "bm-enet" }
    
    --temporary hack
    includedirs { "ext-libs/enet/include" }  
      
  project "sample-client"
    kind "ConsoleApp"
    language "C++"
    
    includedirs { "include/", "src/" }
    files { "src/enet-sample/client.cpp" }
    
    links { "bm-base" }
    links { "bm-enet" }
    
    --temporary hack
    includedirs { "ext-libs/enet/include" }  
