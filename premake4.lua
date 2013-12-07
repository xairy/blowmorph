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

  configuration { "windows" }
    includedirs { "inc/win32" }
    defines { "WIN32", "_WIN32" }
    defines { "_CRT_SECURE_NO_WARNINGS", "_CRT_NONSTDC_NO_DEPRECATE", "_SCL_SECURE_NO_WARNINGS" }

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
      includedirs { "third-party/enet-plus/include" }
      windows_libdir("third-party/enet-plus/bin")
      windows_binary("third-party/enet-plus/bin", "enet-plus.dll")
      links { "enet-plus" }
    configuration "linux"
      links { "enet-plus" }

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
      includedirs { "third-party/enet-plus/include" }
      windows_libdir("third-party/enet-plus/bin")
      windows_binary("third-party/enet-plus/bin", "enet-plus.dll")
      links { "enet-plus" }
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

    defines { "BM_BASE_DLL" }
    includedirs { "src", "inc" }
    files { "src/base/**.cpp",
            "src/base/**.h" }

    -- libconfig
    configuration "windows"
      includedirs { "third-party/libconfig/include" }
      windows_libdir("third-party/libconfig/bin")
      windows_binary("third-party/libconfig/bin", "libconfig_d.dll", "libconfig.dll")
      configuration { "windows", "debug" }
        links { "libconfig_d" }
      configuration { "windows", "release" }
        links { "libconfig" }
    configuration "linux"
      links { "config" }

  project "interpolator"
    kind "StaticLib"
    language "C++"

    includedirs { "src", "inc" }

    files { "src/interpolator/**.cpp",
            "src/interpolator/**.h" }
