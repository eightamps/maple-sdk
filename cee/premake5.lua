
-- Premake build script for Maple SDK project

workspace "maple-sdk"
  configurations { "Debug", "Release" }

  files {
    "src/*.h",
    "src/*.c",
  }

  buildoptions {
      "-std=gnu17",
      "-Wall",
      "-Werror",
  }

  linkoptions {
    "-std=gnu17",
  }

  project "example-gtk"
    kind "ConsoleApp"
    language "C"
    targetdir "dist/%{cfg.buildcfg}"

    buildoptions {
      "`pkg-config --cflags gtk+-3.0`"
    }

    linkoptions {
      "`pkg-config --libs gtk+-3.0`",
    }

    files {
      "src/gtk/*.h",
      "src/gtk/*.c",
      "src/nix/stitch.c",
    }

    -- linkoptions { "-static" }

    links {
      "pthread",
      "m",
      "usb-1.0",
      "soundio",
    }

    removefiles {
      "src/hid_client.c",
      "src/hid_client.h",
      "src/main_console.c",
      "src/main_dll.c",
      "src/main_mingw.c",
      "src/main_sdl2.c",
      "src/sdl/*",
      "src/win32/*",
    }


    buildoptions {
      "--verbose",
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

