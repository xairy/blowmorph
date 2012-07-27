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
        
        includedirs { "include/" }
        
        files { "src/client/**.cpp",
                "src/client/**.hpp",
                "src/base/**.cpp",
                "include/base/**.hpp" }
        
        includedirs { "ext-libs/enet/include" }
        files { "src/enet-wrapper/**.cpp",
                "include/enet-wrapper/**.hpp" }
        links { "enet" }
        
        links { "interpolator" }
        includedirs { "src/interpolator" }
        
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
                    resource(proj, "ext-libs/freetype2/bin/vs2008/" .. confpath .. "/freetype6.dll", "freetype6.dll")
                    resource(proj, "data", "data")
            end
        end
        
    project "interpolator"
        kind "StaticLib"
        language "C++"
        
        targetdir "bin/interpolator"
                      
        includedirs { "include/" }
        
        files { "src/interpolator/**.cpp",
                "src/interpolator/**.hpp",
                "include/base/**.hpp" }
        
        includedirs { "ext-libs/glm/include" }
        
    project "server"
        kind "ConsoleApp"
        language "C++"
        
        targetdir "bin/server"
        
        includedirs { "ext-libs/enet/include" }

        includedirs { "include/" }
        
        files { "src/server/**.cpp",
                "src/server/**.hpp",
                "src/base/**.cpp",
                "include/base/**.hpp" }

        includedirs { "ext-libs/enet/include" }  
        files { "src/enet-wrapper/**.cpp",
                "include/enet-wrapper/**.hpp" }           
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
    
    project "sample-server"
        kind "ConsoleApp"
        language "C++"
        
        targetdir "bin/enet-sample"
        
        includedirs { "include/" }
        
        includedirs { "ext-libs/enet/include" }

        files { "src/enet-wrapper/**.cpp",
                "include/enet-wrapper/**.hpp",
                "src/base/**.cpp",
                "src/enet-sample/server.cpp" }
        
        links { "enet" }
        
        for _,arch in pairs({"x32", "x64"}) do
            for _,conf in pairs({"debug", "release"}) do
                local confpath = arch .. "/" .. conf
                configuration { arch, conf, "vs2008" }
                    libdirs { path.join("ext-libs/enet/bin/vs2008", confpath) }
            end
        end
        
    project "sample-client"
        kind "ConsoleApp"
        language "C++"
        
        targetdir "bin/enet-sample"
        
        includedirs { "include/" }
        
        includedirs { "ext-libs/enet/include" }

        files { "src/enet-wrapper/**.cpp",
                "include/enet-wrapper/**.hpp",
                "src/base/**.cpp",
                "src/enet-sample/client.cpp" }
        
        links { "enet" }
        
        for _,arch in pairs({"x32", "x64"}) do
            for _,conf in pairs({"debug", "release"}) do
                local confpath = arch .. "/" .. conf
                configuration { arch, conf, "vs2008" }
                    libdirs { path.join("ext-libs/enet/bin/vs2008", confpath) }
            end
        end
        
    --[[project "alex-test"
        kind "ConsoleApp"
        language "C++"
        
        targetdir "bin/alex-test"
                      
        includedirs { "include/", "include/alex-test/" }
        
        files { "src/alex-test/**.cpp",
                "src/alex-test/**.hpp" }
        
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
            "freetype2"
          }
        
        for _,arch in pairs({"x32", "x64"}) do
            for _,conf in pairs({"debug", "release"}) do
                local confpath = arch .. "/" .. conf
                configuration { arch, conf, "vs2008" }
                    libdirs { path.join("ext-libs/SDL1.2/bin/vs2008", confpath) }
                    libdirs { path.join("ext-libs/glew/bin/vs2008", confpath) }
                    libdirs { path.join("ext-libs/FreeImage/bin/vs2008", confpath) }
                    libdirs { path.join("ext-libs/freetype2/bin/vs2008", confpath) }
                    
                    local proj = "alex-test"
                    resource(proj, "ext-libs/SDL1.2/bin/vs2008/" .. confpath .. "/SDL.dll", "SDL.dll")
                    resource(proj, "ext-libs/glew/bin/vs2008/" .. confpath .. "/glew32.dll", "glew32.dll")
                    resource(proj, "ext-libs/FreeImage/bin/vs2008/" .. confpath .. "/FreeImage.dll", "FreeImage.dll")
                    resource(proj, "data", "data")
            end
        end
]]--