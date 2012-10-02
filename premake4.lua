function copy(src, dst, always)
  local action = "python "
  local script = "\"" ..  path.join(os.getcwd(), "copy-data.py")  .. "\""
  src = "\"" .. src .. "\""
  dst = "\"" .. dst .. "\""
  cwd = "\"" .. os.getcwd() .. "\""
  postbuildcommands { action .. script .. " " .. cwd .. " " .. src .. " " .. dst .. " " .. tostring(always) }
end

function resource(src, dst, always)
  if always == nil then
    always = false
  else
    always = true
  end

  copy(src, path.join("build", dst), always)
  copy(src, path.join("bin", dst), always)
end

saved_config = {}
function save_config()
  saved_config = configuration().terms
end
function restore_config()
  configuration(saved_config)
end

function windows_libdir(basePath) 
  save_config()

  --XXX: merge configurations?
  for _,arch in pairs({"x32", "x64"}) do
    for _,conf in pairs({"debug", "release"}) do
      for _, plat in pairs({"vs2008"}) do
        local confpath = plat .. "/" .. arch .. "/" .. conf
        configuration { "windows", arch, conf, plat }
          libdirs { path.join(basePath, confpath) }
      end
    end
  end
  
  restore_config()
end

function windows_binary(basePath, dllName)
  save_config()

  for _,arch in pairs({"x32", "x64"}) do
    for _,conf in pairs({"debug", "release"}) do
      for _, plat in pairs({"vs2008"}) do
        local confpath = plat .. "/" .. arch .. "/" .. conf
        configuration { "windows", arch, conf, plat }
          resource(path.join(path.join(basePath, confpath), dllName), dllName, true)
      end
    end
  end
  
  restore_config()
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
  
  flags { "FatalWarnings", "NoRTTI" }
  
  configuration { "linux" }
    flags { "NoExceptions" }
  
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
    
    includedirs { "src", "inc" }
    files { "src/client/**.cpp",
            "src/client/**.hpp" }
    
    links { "bm-base" }
    links { "bm-enet" }
    links { "interpolator" }
    
    links { "IniFile" }    
    
    resource("data", "data")
      
    -- GLM
    includedirs { "ext-libs/glm/include" }
      
    -- OpenGL
    configuration "windows"
      links { "opengl32" }
      links { "glu32" }
      
    -- FreeType2
    configuration "windows"
      includedirs { "ext-libs/freetype2/include" }
      windows_libdir("ext-libs/freetype2/bin")
      windows_binary("ext-libs/freetype2/bin", "freetype.dll")
      links { "freetype" }
    configuration "linux"
      includedirs { "/usr/include/freetype2" }
      links { "freetype" }
      
    -- FreeImage
    configuration "windows"
      includedirs { "ext-libs/FreeImage/include" }
      windows_libdir("ext-libs/FreeImage/bin")
      windows_binary("ext-libs/FreeImage/bin", "FreeImage.dll")
      links { "freeimage" }
    configuration "linux"
      links { "freeimage" }
    
    -- GLEW
    configuration { "windows" }
      includedirs { "ext-libs/glew/include" }
      windows_libdir("ext-libs/glew/bin")
      windows_binary("ext-libs/glew/bin", "glew32.dll")
      links { "glew32" }
    configuration "linux"
      links { "GLEW" }
    
    -- SDL1.2
    configuration "windows"
      includedirs { "ext-libs/SDL1.2/include" }
      windows_libdir("ext-libs/SDL1.2/bin")
      windows_binary("ext-libs/SDL1.2/bin", "SDL.dll")
      links { "SDLmain" }
      links { "SDL" }
    configuration "linux"
      links { "SDLmain" }
      links { "SDL" }
    
    -- PugiXML
    configuration "windows"
      includedirs { "ext-libs/pugixml/include" }
      windows_libdir("ext-libs/pugixml/bin")
      links { "pugixml" }
    configuration "linux"
      links { "pugixml" }
  
  project "server"
    kind "ConsoleApp"
    language "C++"
    
    includedirs { "src", "inc" }
    files { "src/server/**.cpp",
            "src/server/**.hpp" }
    
    links { "bm-base" }
    links { "bm-enet" }

    resource("data", "data")
    
    -- PugiXML
    configuration "windows"
      includedirs { "ext-libs/pugixml/include" }
      windows_libdir("ext-libs/pugixml/bin")
      links { "pugixml" }
    configuration "linux"
      links { "pugixml" }
  
  project "sample-server"
    kind "ConsoleApp"
    language "C++"
    
    includedirs { "src", "inc" }
    files { "src/enet-sample/server.cpp" }
    
    links { "bm-base" }
    links { "bm-enet" }
      
  project "sample-client"
    kind "ConsoleApp"
    language "C++"
    
    includedirs { "src", "inc" }
    files { "src/enet-sample/client.cpp" }
    
    links { "bm-base" }
    links { "bm-enet" }
  
  project "bm-base"
    kind "SharedLib"
    language "C++"
    
    defines { "BM_BASE_DLL" }
    includedirs { "src", "inc" }
    files { "src/base/**.cpp",
            "src/base/**.hpp" }

  project "bm-enet"
    kind "SharedLib"
    language "C++"
      
    defines { "BM_ENET_DLL" }
    includedirs { "src", "inc" }
    files { "src/enet-wrapper/**.cpp",
            "src/enet-wrapper/**.hpp" }
     
    links { "bm-base" }
        
    -- ENet
    configuration "windows"
      includedirs { "ext-libs/enet/include" }      
      windows_libdir("ext-libs/enet/bin")
      windows_binary("ext-libs/enet/bin", "enet.dll")
      links { "enet" }
      links { "ws2_32", "winmm" }
    configuration "linux"
      links { "enet" }
    
  project "interpolator"
    kind "StaticLib"
    language "C++"
                  
    includedirs { "src", "inc" }
    
    files { "src/interpolator/**.cpp",
            "src/interpolator/**.hpp" }

  project "IniFile"
    kind "SharedLib"
    language "C++"
    
    defines "INIFILE_DLL"
    
    includedirs { "src", "inc" }
    
    files { "src/IniFile/**.cpp",
            "src/IniFile/**.hpp" }