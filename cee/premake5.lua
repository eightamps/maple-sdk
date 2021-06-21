
-- Premake build script for Maple SDK project

workspace "maple-sdk"
  configurations { "Debug", "Release" }

project "example-gtk"
  kind "ConsoleApp"
  language "C"
  buildoptions { "-std=C11" }
  targetdir "dist/%{cfg.buildcfg}"

  -- local gtkcflags = "`pkg-config --cflags gtk+-3.0`"
  -- local gtklibs = "`pkg-config --libs gtk+-3.0`"
  local gtkcflags = "-pthread -I/usr/include/gtk-3.0 -I/usr/include/at-spi2-atk/2.0 -I/usr/include/at-spi-2.0 -I/usr/include/dbus-1.0 -I/usr/lib/x86_64-linux-gnu/dbus-1.0/include -I/usr/include/gtk-3.0 -I/usr/include/gio-unix-2.0 -I/usr/include/cairo -I/usr/include/pango-1.0 -I/usr/include/fribidi -I/usr/include/harfbuzz -I/usr/include/atk-1.0 -I/usr/include/cairo -I/usr/include/pixman-1 -I/usr/include/uuid -I/usr/include/freetype2 -I/usr/include/libpng16 -I/usr/include/gdk-pixbuf-2.0 -I/usr/include/libmount -I/usr/include/blkid -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include"
  print("GTK CFLAGS: " .. gtkcflags)
  local gtklibs = "gtk-3 -lgdk-3 -lpangocairo-1.0 -lpango-1.0 -lharfbuzz -latk-1.0 -lcairo-gobject -lcairo -lgdk_pixbuf-2.0 -lgio-2.0 -lgobject-2.0 -lglib-2.0"
  print("GTK LIBS: " .. gtklibs)

  buildoptions {
    gtkcflags
  }

  links {
    "pthread",
    "m",
    gtklibs
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

