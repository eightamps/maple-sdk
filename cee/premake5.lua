
-- Premake build script for Maple SDK project

workspace "maple-sdk"
  configurations { "Debug", "Release" }
  language "C"

  buildoptions {
      "-std=gnu17",
  }

  linkoptions {
    "-std=gnu17",
  }

  links {
    "pthread",
    "m",
  }

  -- Shared project implementations and headers
  files {
    -- "src/hid_client.h", -- This isn't quite ready yet
    -- "src/hid_client.c",
    "src/dtmf.c",
    "src/dtmf.h",
    "src/log.h",
    "src/phony.c",
    "src/phony.h",
    "src/phony_hid.c",
    "src/phony_hid.h",
    "src/shared.h",
    "src/stitch.h",
  }

  filter "configurations:Debug"
    defines { "DEBUG" }
    symbols "On"

  filter "configurations: Release"
    defines { "NDEBUG" }
    optimize "On"

  -- GTK Example Binary
  project "example-gtk"
    kind "ConsoleApp"
    targetdir "dist/%{cfg.buildcfg}"

    buildoptions {
      "`pkg-config --cflags gtk+-3.0`",
      "-Wall",
      "-Werror",
    }

    linkoptions {
      "`pkg-config --libs gtk+-3.0`",
    }

    links {
      "usb-1.0",
      "soundio",
    }

    files {
      "src/gtk/*.c",
      "src/gtk/*.h",
      "src/main_gtk.c",
      "src/nix/stitch.c",
    }


  -- Nix Test Runner
  project "nix-test"
    kind "ConsoleApp"
    targetdir "dist/%{cfg.buildcfg}"
    symbols "On"
    optimize "Off"

    links {
      "usb-1.0",
      "soundio",
    }

    files {
      "src/nix/stitch.c",
      "test/*.h",
      "test/*.c",
      "test/fakes/*.h",
      "test/fakes/*.c",
      "test/main_test.c",
    }

    -- Tests to skip for now...
    removefiles {
      "test/hid_client_test.*",
      "test/kissfft_test.*",
      "test/phony_view_test.*",
    }


