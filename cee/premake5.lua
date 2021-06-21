
-- ##############################################
-- Premake build script for Maple SDK project
-- ##############################################

workspace "maple-sdk"
  configurations { "Debug", "Release" }
  language "C"

  platforms {
    "linux64",
    "linux86",
    "win32",
    "win64",
  }

  buildoptions {
    -- "-std=gnu17",
  }

  linkoptions {
    -- "-std=gnu17",
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

  -- ##########################################
  -- Win32 Executable example
  -- clang -target i686-pc-windows-gnu src/main_win32.c -otest.exe
  -- ##########################################

  project "ExampleWin32"
    kind "ConsoleApp"
    targetdir "dist/%{cfg.buildcfg}"

    files {
      "src/main_console.c",
    }

    buildoptions {
      "-target i686-pc-windows-gnu",
      "-v",
    }

    linkoptions {
      "-target i686-pc-windows-gnu",
      "-v",
    }

    includedirs {
      "/usr/include/x86_64-linux-musl/",
      "./vendor/libusb/include",
    }

    libdirs {
      "/usr/lib/x86_64-linux-musl/",
      -- "vendor/libusb/VS2019/MS32/static",
      -- "./vendor/libusb/MinGW32/static",
    }

    links {
      "c",

      -- "vendor/libusb/MinGW32/static/libusb-1.0.a",
    }

  -- ##########################################
  -- Win32 Shared Library
  -- ##########################################
  --filter { "platforms:win32" }
  --  defines { "WIN32" }

  --  project "MapleSdk"
  --    kind "SharedLib"
  --    targetdir "dist/%{cfg.buildcfg}"

  --    buildoptions {
  --      "-Wall",
  --      "-Werror",
  --    }

  --    linkoptions {
  --      -- "-fPIC",
  --      -- "-Wl,-z,notext",
  --    }

  --    links {
  --      "usb-1.0",
  --      "soundio",
  --    }

  -- ##########################################
  -- Nix Shared and Static Library
  -- ##########################################
  -- project "maple-sdk-shared"
  --   kind "SharedLib"
  --   targetdir "dist/%{cfg.buildcfg}"

  --   buildoptions {
  --     "-Wall",
  --     "-Werror",
  --   }

  --   links {
  --     "usb-1.0",
  --     "soundio",
  --   }

  -- project "maple-sdk-static"
  --   kind "StaticLib"
  --   targetdir "dist/%{cfg.buildcfg}"

  --   links {
  --     "usb-1.0",
  --     "soundio",
  --   }

  -- ##########################################
  -- Nix Console Example
  -- ##########################################
  -- project "example-console"
  --   kind "ConsoleApp"
  --   targetdir "dist/%{cfg.buildcfg}"

  --   buildoptions {
  --     "-Wall",
  --     "-Werror",
  --   }

  --   links {
  --     "usb-1.0",
  --     "soundio",
  --   }

  --   files {
  --     "src/main_console.c",
  --     "src/nix/stitch.c",
  --   }

  -- -- ##########################################
  -- -- GTK Example Binary
  -- -- ##########################################
  -- project "example-gtk"
  --   kind "WindowedApp"
  --   targetdir "dist/%{cfg.buildcfg}"

  --   buildoptions {
  --     "`pkg-config --cflags gtk+-3.0`",
  --     "-Wall",
  --     "-Werror",
  --   }

  --   linkoptions {
  --     "`pkg-config --libs gtk+-3.0`",
  --   }

  --   links {
  --     "usb-1.0",
  --     "soundio",
  --   }

  --   files {
  --     "src/gtk/*.c",
  --     "src/gtk/*.h",
  --     "src/main_gtk.c",
  --     "src/nix/stitch.c",
  --   }

  -- -- ##########################################
  -- -- Test Runner
  -- -- ##########################################
  -- project "test"
  --   kind "ConsoleApp"
  --   targetdir "dist/%{cfg.buildcfg}"
  --   symbols "On"
  --   optimize "Off"

  --   links {
  --     "usb-1.0",
  --     "soundio",
  --   }

  --   files {
  --     "src/nix/stitch.c",
  --     "test/*.h",
  --     "test/*.c",
  --     "test/fakes/*.h",
  --     "test/fakes/*.c",
  --     "test/main_test.c",
  --   }

  --   -- Tests to skip for now...
  --   removefiles {
  --     "test/hid_client_test.*",
  --     "test/kissfft_test.*",
  --     "test/phony_view_test.*",
  --   }


