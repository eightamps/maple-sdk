
-- ##############################################
-- Premake build script for Maple SDK project
-- ##############################################

workspace "maple-sdk"
  configurations { "Debug", "Release" }
  language "C"

  buildoptions {
     "-std=c11",
  }

  linkoptions {
     "-std=c11",
  }

  links {
    "pthread",
    "m",
  }

  -- Shared project implementations and headers
  files {
    -- "src/hid_client.h", -- This isn't quite ready yet
    "src/hid_client.c",
    "src/hid_client.h",
    "src/hid_status.c",
    "src/hid_status.h",
    "src/infrareddy_hid.c",
    "src/infrareddy_hid.h",
    "src/dtmf.c",
    "src/dtmf.h",
    "src/log.h",
    "src/phony*",
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

    local win_target = "-target i686-w64-mingw32"
    -- local win_target = "-target i686-pc-win32 -fms-extensions"

    local libusb_header = "-Ivendor/libusb/include"
    local libusb_dir = "vendor/libusb/MinGW32/static"
    local libusb_file = "usb-1.0"

    files {
      -- "src/main_win32.c",
      "src/main_console.c",
      "src/win32/stitch.c",
    }

    buildoptions {
      -- "-std=c11",
      "-v",
      "-DWIN32_LEAN_ANDMEAN",
      win_target,
      libusb_header,
    }

    linkoptions {
      -- "-std=c11",
      "-static",
      "-v",
      win_target,
      libusb_header,
      "-L" .. libusb_dir,
      "-l" .. libusb_file,
    }

    links {
      "ole32",
    }

  -- ##########################################
  -- Win32 Shared Library
  -- ##########################################
  -- project "MapleSdkWin32"
  --   kind "SharedLib"
  --   targetdir "dist/%{cfg.buildcfg}"

  --   local win_target = "-target i686-w64-mingw32"
  --   -- local win_target = "-target i686-pc-win32 -fms-extensions"

  --   local libusb_header = "-Ivendor/libusb/include"
  --   local libusb_dir = "vendor/libusb/MinGW32/static"
  --   local libusb_file = "usb-1.0"

  --   files {
  --     -- "src/main_win32.c",
  --     "src/main_console.c",
  --     "src/win32/stitch.c",
  --   }

  --   buildoptions {
  --     -- "-std=c11",
  --     "-v",
  --     "-DWIN32_LEAN_ANDMEAN",
  --     win_target,
  --     libusb_header,
  --   }

  --   linkoptions {
  --     -- "-std=c11",
  --     "-static",
  --     "-v",
  --     win_target,
  --     libusb_header,
  --     "-L" .. libusb_dir,
  --     "-l" .. libusb_file,
  --   }

  --   links {
  --     "ole32",
  --   }

  -- filter { "platforms:win32" }
  --   defines { "WIN32" }

  --   project "MapleSdk"
  --     kind "SharedLib"
  --     targetdir "dist/%{cfg.buildcfg}"

  --     buildoptions {
  --       "-Wall",
  --       "-Werror",
  --     }

  --     links {
  --       "usb-1.0",
  --       "soundio",
  --     }

  -- ##########################################
  -- Nix Shared and Static Library
  -- ##########################################
  project "maple-sdk-shared"
    kind "SharedLib"
    targetdir "dist/%{cfg.buildcfg}"

    buildoptions {
      "-Wall",
      "-Werror",
    }

    links {
      "usb-1.0",
      "soundio",
    }

  project "maple-sdk-static"
    kind "StaticLib"
    targetdir "dist/%{cfg.buildcfg}"

    links {
      "usb-1.0",
      "soundio",
    }

  -- ##########################################
  -- GTK Example Binary
  -- ##########################################
  project "example-gtk"
    kind "WindowedApp"
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

    includedirs {
      "vendor/libsoundio/include",
      "vendor/libusb/include",
    }

    files {
      "src/gtk/*.c",
      "src/gtk/*.h",
      "src/main_gtk.c",
      "src/nix/*.c",
      "src/infrareddy*",
      "src/phony*",
    }

  -- ##########################################
  -- Test Runner
  -- ##########################################
  project "maple-sdk-test"
    kind "ConsoleApp"
    targetdir "dist/%{cfg.buildcfg}"
    symbols "On"
    optimize "Off"

    buildoptions {
      -- "-v",
      "-DTEST_MODE=1",
      "-Itest/fakes",
    }

    linkoptions {
      -- "-v",
    }

    files {
      "test/fakes/**/*.h",
      "test/fakes/**/*.c",
      "src/nix/stitch.c",
      "src/nix/stitch_soundio.h",
      "test/*.h",
      "test/*.c",
      "test/main_test.c",
    }

    -- Tests to skip for now...
    removefiles {
      "test/hid_client_test.*",
      "test/kissfft_test.*",
      "test/phony_view_test.*",
    }


