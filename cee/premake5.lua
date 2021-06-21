
-- Premake build script for Maple SDK project

local pkgconfig = require 'vendor/premake-pkgconfig/pkgconfig'

workspace "maple-sdk"
  configurations { "Debug", "Release" }

project "example-gtk"
  kind "ConsoleApp"
  language "C"
  targetdir "dist/%{cfg.buildcfg}"

  -- local atk = pkgconfig.load("atk")
  -- local gdk = pkgconfig.load("gdk-pixbuf-2.0")
  -- local cairo = pkgconfig.load("cairo")
  -- local harfbuzz = pkgconfig.load("harfbuzz")
  -- local pango = pkgconfig.load("pango")
  -- local glib = pkgconfig.load("glib-2.0")
  local gtk = pkgconfig.parse(pkgconfig.load("gtk+-3.0"))


  -- local pkgs = pkgconfig.parse(atk .. gdk .. cairo .. harfbuzz .. pango .. glib .. gtk)

  print("gtk.CFLAGS: " .. gtk.cflags)
  print("gtk.LIBS: " .. gtk.libs)

  buildoptions {
    gtk.cflags
  }

  links {
    string.sub(gtk.libs, 3)
  }

  files {
    "src/*.h",
    "src/*.c",
    "src/gtk/*.h",
    "src/gtk/*.c"
  }

  removefiles {
    "src/main_console.c",
    "src/main_dll.c",
    "src/main_mingw.c",
    "src/main_sdl2.c",
    "src/sdl/*",
    "src/win32/*"
  }


  buildoptions {
    "--verbose"
  }

  -- filter { "system:linux", "options:linux_backend=gtk3" }
    -- language "C"
    -- buildoptions { "`pkg-config --libs --cflags gtk+-3.0`" }

  filter "configurations:Debug"
    defines { "DEBUG" }
    symbols "On"

  filter "configurations: Release"
    defines { "NDEBUG" }
    optimize "On"

