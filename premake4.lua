function copy(src, dst)
  local action = "\"" ..  path.join(os.getcwd(), "copy-data.py")  .. "\""
  src = "\"" .. src .. "\""
  dst = "\"" .. dst .. "\""
  cwd = "\"" .. os.getcwd() .. "\""
  postbuildcommands { action .. " " .. cwd .. " " .. src .. " " .. dst }
end

function resource(proj, src, dst)
  copy(src, path.join("build", dst))
  if proj == nil then
    copy(src, path.join("bin", dst))
  else
    copy(src, path.join(path.join("bin", proj), dst))
  end
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
        
        targetdir "bin/client"
        
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
        
        for _,arch in pairs({"x32", "x64"}) do
            for _,conf in pairs({"debug", "release"}) do
                local confpath = arch .. "/" .. conf
                configuration { arch, conf, "vs2008" }
                    libdirs { path.join("ext-libs/SDL1.2/bin/vs2008", confpath) }
                    libdirs { path.join("ext-libs/glew/bin/vs2008", confpath) }
                    libdirs { path.join("ext-libs/enet/bin/vs2008", confpath) }
                    libdirs { path.join("ext-libs/FreeImage/bin/vs2008", confpath) }
                    libdirs { path.join("ext-libs/freetype2/bin/vs2008", confpath) }
                    
                    local proj = "client"
                    resource(proj, "ext-libs/SDL1.2/bin/vs2008/" .. confpath .. "/SDL.dll", "SDL.dll")
                    resource(proj, "ext-libs/glew/bin/vs2008/" .. confpath .. "/glew32.dll", "glew32.dll")
                    resource(proj, "ext-libs/FreeImage/bin/vs2008/" .. confpath .. "/FreeImage.dll", "FreeImage.dll")
                    resource(proj, "ext-libs/freetype2/bin/vs2008/" .. confpath .. "/freetype.dll", "freetype.dll")
                    resource(proj, "data", "data")
            end
        end
    
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
        
        targetdir "bin/interpolator"
                      
        includedirs { "include/", "src/" }
        
        files { "src/interpolator/**.cpp",
                "src/interpolator/**.hpp" }
        
    project "server"
        kind "ConsoleApp"
        language "C++"
        
        targetdir "bin/server"
        
        includedirs { "include/", "src/" }
        files { "src/server/**.cpp",
                "src/server/**.hpp" }
        
        links { "bm-base" }
        links { "bm-enet" }
        
        --temporary hack
        includedirs { "ext-libs/enet/include" }  
    
    project "sample-server"
        kind "ConsoleApp"
        language "C++"
        
        targetdir "bin/enet-sample"
        
        includedirs { "include/", "src/" }
        files { "src/enet-sample/server.cpp" }
        
        links { "bm-base" }
        links { "bm-enet" }
        
        --temporary hack
        includedirs { "ext-libs/enet/include" }  
        
    project "sample-client"
        kind "ConsoleApp"
        language "C++"
        
        targetdir "bin/enet-sample"
        
        includedirs { "include/", "src/" }
        files { "src/enet-sample/client.cpp" }
        
        links { "bm-base" }
        links { "bm-enet" }
        
        --temporary hack
        includedirs { "ext-libs/enet/include" }  