
-- ##############################################
-- Premake build script for Maple SDK project
-- ##############################################

workspace "maple-sdk"
  configurations { "Debug", "Release" }
  language "C"

  -- platforms {
    -- "linux64",
    -- "linux86",
    -- "win32",
    -- "win64",
  -- }

  -- buildoptions {
    -- "-std=gnu17",
  -- }

  -- linkoptions {
    -- "-std=gnu17",
  -- }

  links {
    "pthread",
    "m",
  }

  -- Shared project implementations and headers
  files {
    -- "src/hid_client.h", -- This isn't quite ready yet
    -- "src/hid_client.c",
    -- "src/dtmf.c",
    -- "src/dtmf.h",
    -- "src/log.h",
    -- "src/phony*",
    -- "src/shared.h",
    -- "src/stitch.h",
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

    -- local libusb = "vendor/libusb/MinGW32/static/libusb-1.0.a"
    -- local libusb_dir = "vendor/libusb/MinGW32/dll"

    -- FAILS to find libusb.a
    -- local libusb_dir = "vendor/libusb/MinGW32/static"
    -- local libusb_file = "libusb-1.0.a"

    -- FINDS the libusb.dll, but fails to find symbols within it
    local libusb_header = "-Ivendor/libusb/include"
    local libusb_dir = "vendor/libusb/MinGW32/static"
    local libusb_file = "usb-1.0"

    -- FINDS the libusb.dll, but fails to find symbols within it
    -- local libusb_dir = "vendor/libusb/MinGW32/dll"
    -- local libusb_file = "libusb-1.0.dll"

    -- FINDS the libusb.dll, but fails to find symbols winthin it
    -- local libusb_dir = "vendor/libusb/VS2019/MS64/dll"
    -- local libusb_file = "libusb-1.0.dll"

    files {
      -- "src/main_win32.c",
      "src/main_console.c",
      "src/win32/stitch.c",
      "src/dtmf.c",
      "src/dtmf.h",
      "src/log.h",
      "src/shared.h",
      "src/stitch.h",
      "src/phony*",
    }

    buildoptions {
      -- "-std=c11",
      "-v",
      "-DWIN32_LEAN_ANDMEAN",
      win_target,
      libusb_header,
      -- "-target i686-w64-mingw32",
      -- "-target x86_64-w64-mingw32",
      -- "-target i686-pc-windows-gnu",
    }

    linkoptions {
      -- "-std=c11",
      "-static",
      "-v",
      win_target,
      libusb_header,
      "-L" .. libusb_dir,
      "-l" .. libusb_file,
      -- "-static-libwinpthread",
      -- "-target i686-w64-mingw32",
      -- "-target i686-pc-windows-gnu",
      -- "-Bstatic",
      -- "-Lvendor/libusb/MinGW32/static",
      -- "-Lvendor/libusb/MinGW32/static/libusb-1.0.a",
    }

    includedirs {
      -- "vendor/libusb/include",
    }

    libdirs {
      -- "vendor/libusb/VS2019/MS32/static",
      -- "vendor/libusb/VS2019/MS64/static",
      -- "vendor/libusb/MinGW32/static",
      -- "vendor/libusb/VS2019/MS32/static",
    }

    links {
      "ole32",
      -- libusb_dir .. "/" .. libusb_file,
      -- libusb_file,
      -- "vendor/libusb/VS2019/MS32/dll/libusb-1.0.dll",
      -- "vendor/libsoundio/win32/soundio",
      -- "usb-1.0.dll",
      -- "libusb-1.0",
    }

  -- ##########################################
  -- Win32 Shared Library
  -- ##########################################
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


