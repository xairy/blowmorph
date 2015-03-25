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

function windows_binary(basePath, debugDllName, releaseDllName)
  if releaseDllName == nil then
    releaseDllName = debugDllName
  end

  save_config()

  for _,arch in pairs({"x32", "x64"}) do
    for _, plat in pairs({"vs2008"}) do
      local confpath = plat .. "/" .. arch .. "/" .. "debug"
      configuration { "windows", arch, "debug", plat }
        resource(path.join(path.join(basePath, confpath), debugDllName), debugDllName, true)
      local confpath = plat .. "/" .. arch .. "/" .. "release"
      configuration { "windows", arch, "release", plat }
        resource(path.join(path.join(basePath, confpath), releaseDllName), releaseDllName, true)
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
    -- TODO: use "-stdlib=libc++" for clang.
    linkoptions { "-std=c++11" }
    buildoptions { "-std=c++11" }

  configuration { "windows" }
    -- TODO: C++11
    includedirs { "src/windows" }
    defines { "WIN32", "_WIN32" }
    defines { "_CRT_SECURE_NO_WARNINGS", "_CRT_NONSTDC_NO_DEPRECATE", "_SCL_SECURE_NO_WARNINGS" }

  configuration { "debug" }
    defines { "DEBUG" }
    flags { "Symbols" }

  configuration { "release" }
    defines { "NDEBUG" }
    flags { "Optimize" }

  project "base"
    kind "SharedLib"
    language "C++"

    defines { "BM_BASE_DLL" }
    includedirs { "src" }
    files { "src/base/**.cpp",
            "src/base/**.h" }

    -- JsonCpp
    configuration "windows"
      -- TODO
    configuration "linux"
      links { "jsoncpp" }

  project "net"
    kind "SharedLib"
    language "C++"

    defines { "BM_NET_DLL" }
    includedirs { "src" }
    files { "src/net/**.cpp",
            "src/net/**.h" }

    links { "base" }

    configuration "linux"
      links { "enet" }
    configuration "windows"
      includedirs { "third-party/enet/include" }      
      windows_libdir("third-party/enet/bin")
      links { "ws2_32", "winmm" }
      links { "enet" }

  project "engine"
    kind "SharedLib"
    language "C++"

    defines { "BM_ENGINE_DLL" }
    includedirs { "src" }
    files { "src/engine/**.cpp",
            "src/engine/**.h" }

    links { "base" }

    -- JsonCpp
    configuration "windows"
      -- TODO
    configuration "linux"
      links { "jsoncpp" }

    -- Box2D
    configuration "windows"
      -- TODO
    configuration "linux"
      links { "Box2D" }

  project "server"
    kind "ConsoleApp"
    language "C++"
    targetname "server"

    includedirs { "src" }
    files { "src/server/**.cpp",
            "src/server/**.h" }

    links { "base", "engine", "net" }

    configuration "windows"
      resource("data", "data")

    -- Box2D
    configuration "windows"
      -- TODO
    configuration "linux"
      links { "Box2D" }

  project "client"
    kind "ConsoleApp"
    language "C++"
    targetname "client"

    includedirs { "src" }
    files { "src/client/**.cpp",
            "src/client/**.h" }

    links { "base", "engine", "net" }

    configuration "windows"
      resource("data", "data")

    -- SFML
    configuration "windows"
      includedirs { "third-party/SFML/include" }
      windows_libdir("third-party/SFML/bin")
      windows_binary("third-party/SFML/bin", "sfml-system-d-2.dll", "sfml-system-2.dll")
      windows_binary("third-party/SFML/bin", "sfml-window-d-2.dll", "sfml-window-2.dll")
      windows_binary("third-party/SFML/bin", "sfml-graphics-d-2.dll", "sfml-graphics-2.dll")
      windows_binary("third-party/SFML/bin", "sfml-audio-d-2.dll", "sfml-audio-2.dll")
      configuration { "windows", "debug" }
        links { "sfml-system-d" }
        links { "sfml-window-d" }
        links { "sfml-graphics-d" }
        links { "sfml-audio-d" }
      configuration { "windows", "release" }
        links { "sfml-system" }
        links { "sfml-window" }
        links { "sfml-graphics" }
        links { "sfml-audio" }
    configuration "linux"
      links { "sfml-system" }
      links { "sfml-window" }
      links { "sfml-graphics" }
      links { "sfml-audio" }

    -- Box2D
    configuration "windows"
      -- TODO
    configuration "linux"
      links { "Box2D" }

  project "interpolator"
    kind "StaticLib"
    language "C++"

    includedirs { "src" }

    files { "src/interpolator/**.cpp",
            "src/interpolator/**.h" }
