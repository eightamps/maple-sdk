
-- Premake build script for Maple SDK project

workspace "maple-sdk"
  configurations { "Debug", "Release" }

project "sdk-example"
  kind "ConsoleApp"
  language "C"
  targetdir "dist/%{cfg.buildcfg}"

  files {
    "src/**.h",
    "src/**.c",
    "src/gtk/*.h",
    "src/gtk/*.c",
  }

  filter "configurations:Debug"
    defines { "DEBUG" }
    symbols "On"

  filter "configurations: Release"
  defines { "NDEBUG" }
  optimize "On"

