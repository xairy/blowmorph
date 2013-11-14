function copy(src, dst, always)
  local action = "python"
  local script = "\"" ..  path.join(os.getcwd(), "copy-data.py")  .. "\""
  src = "\"" .. src .. "\""
  dst = "\"" .. dst .. "\""
  cwd = "\"" .. os.getcwd() .. "\""
  postbuildcommands { action .. " " .. script .. " " .. cwd .. " " .. src .. " " .. dst .. " " .. tostring(always) }
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
    includedirs { "inc/win32" }
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
    targetname "bm-client"

    includedirs { "src", "inc" }
    files { "src/client/**.cpp",
            "src/client/**.h" }

    links { "bm-base" }
    links { "interpolator" }

    resource("data", "data")

    -- ENetPlus
    configuration "windows"
      -- TODO
    configuration "linux"
      links { "enet-plus" }

    -- XXX: is it required?
    -- OpenGL
    configuration "windows"
      links { "opengl32" }
      links { "glu32" }
    configuration "linux"
      links { "GL" }

    -- SFML
    configuration "windows"
      -- TODO
    configuration "linux"
      links { "sfml-system" }
      links { "sfml-window" }
      links { "sfml-graphics" }
      links { "sfml-audio" }

    -- PugiXML
    configuration "windows"
      includedirs { "third-party/pugixml/include" }
      windows_libdir("third-party/pugixml/bin")
      links { "pugixml" }
    configuration "linux"
      links { "pugixml" }

  project "server"
    kind "ConsoleApp"
    language "C++"
    targetname "bm-server"

    includedirs { "src", "inc" }
    files { "src/server/**.cpp",
            "src/server/**.h" }

    links { "bm-base" }

    resource("data", "data")

    -- ENetPlus
    configuration "windows"
      -- TODO
    configuration "linux"
      links { "enet-plus" }

    -- PugiXML
    configuration "windows"
      includedirs { "third-party/pugixml/include" }
      windows_libdir("third-party/pugixml/bin")
      links { "pugixml" }
    configuration "linux"
      links { "pugixml" }

  project "bm-base"
    kind "SharedLib"
    language "C++"

    -- libconfig
    configuration "windows"
      -- TODO
    configuration "linux"
      links { "config" }

    defines { "BM_BASE_DLL" }
    includedirs { "src", "inc" }
    files { "src/base/**.cpp",
            "src/base/**.h" }

  project "interpolator"
    kind "StaticLib"
    language "C++"

    includedirs { "src", "inc" }

    files { "src/interpolator/**.cpp",
            "src/interpolator/**.h" }
