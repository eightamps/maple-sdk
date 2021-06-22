
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
		"-fuse-ld=lld",
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
	-- Example make task that also doesn't work
  -- crap:
	  -- zig cc \
		-- -std=c11 \
		-- -z std=c11 \
		-- --verbose \
		-- --target=x86_64-windows-gnu \
		-- src/dtmf.* \
		-- src/log.h src/phony* \
		-- src/shared.h \
		-- src/stitch.h \
		-- src/main_win32.c \
		-- -lvendor/pthread-win32/lib/x86/pthreadVSE2 \
		-- -lvendor/libusb/VS2019/MS32/static/libusb-1.0\
		-- -lvendor/libsoundio/lib/libsoundio-win32 \
		-- -lkernel32 \
		-- -luser32 \
		-- -lgdi32 \
		-- -lole32 \
		-- -ld3d11 \
		-- -ldxgi \
		-- -Ivendor/pthread-win32/include \
		-- -Isrc/win32 \
		-- -Ivendor/libusb/include \
		-- -o dist/Debug/main_win.exe

  project "ExampleWin32"
    kind "ConsoleApp"
    targetdir "dist/%{cfg.buildcfg}"

    files {
      -- "src/main_console.c",
			"src/main_win32.c",
    }

    buildoptions {
			"-target x86_64-windows-gnu",
      --"-target i686-pc-windows-gnu",
      "-v",
    }

    linkoptions {
			"-lc",
			"-target x86_64-windows-gnu",
      --"-target i686-pc-windows-gnu",
    }

    includedirs {
      "/usr/include/x86_64-linux-musl/",
      -- "./vendor/libusb/include",
    }

    libdirs {
      "/usr/lib/x86_64-linux-musl/",
      -- "vendor/libusb/VS2019/MS32/static",
      -- "./vendor/libusb/MinGW32/static",
    }

    links {
      -- "c",

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


