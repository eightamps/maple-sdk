cmake_minimum_required(VERSION 3.19)
project(maple C)

set(CMAKE_C_STANDARD 11)

include_directories(src)
include_directories(test)
include_directories(vendor/kissfft)

set(AppSources
        src/dtmf.c
        src/dtmf.h
        src/hid_client.c
        src/hid_client.h
        src/phony.c
        src/phony.h
        src/phony_hid.c
        src/phony_hid.h
        src/stitch.h
        )

set(GtkSources
        src/gtk/application.c
        src/gtk/application.h
        src/gtk/phony_view.c
        src/gtk/phony_view.h
        )

set(TestSources
        test/dtmf_test.c
        test/dtmf_test.h
        test/kissfft_test.c
        test/kissfft_test.h
        test/phony_view_test.c
        test/phony_view_test.h
        test/phony_hid_test.c
        test/phony_hid_test.h
        test/phony_test.c
        test/phony_test.h
        test/test_helper.c
        test/test_helper.h
        test/stitch_test.c
        test/stitch_test.h
        test/hid_client_test.c
        test/hid_client_test.h
        test/fakes/libusb_fake.c
        # test/fakes/libusb_fake.h
        )

set(VendorLibs
        ${PROJECT_SOURCE_DIR}/vendor/kissfft/libkissfft-float.so
        )

# file(COPY "${PROJECT_SOURCE_DIR}/src/sdl/ttf"
        # DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}src/sdl)

# FFT library will be used to validate generated DTMF signals
# link_directories(${CMAKE_BINARY_DIR}}/vendor/kissfft)

# Build environment for Windows DLL
if (WIN32_FALSE)
    set(CMAKE_SYSTEM_NAME Windows)
    set(CMAKE_C_COMPILER i686-w64-mingw32-gcc)
    set(CMAKE_CXX_COMPILER i686-w64-mingw32-g++)
    set(CMAKE_RC_COMPILER i686-w64-mingw32-windres)
    set(CMAKE_RANLIB i686-w64-mingw32-ranlib)

    set(AppSources ${AppSources}
            src/win32/stitch.c
            )

    set(VendorLibs
            ${VendorLibs}
            # Custom built soundio for win32 only
            # ${PROJECT_SOURCE_DIR}/vendor/libsoundio/lib/libsoundio-win32.a
            # ${PROJECT_SOURCE_DIR}/vendor/libsoundio/lib/libsoundio-win32.so
            )

    link_libraries(
            m
            soundio
            usb-1.0
            pthread
    )

    add_library(maple-sdk.dll STATIC ${AppSources} ${VendorLibs})

    set_target_properties(maple-sdk.dll
            PROPERTIES
            PREFIX ""
            SUFFIX ""
            LINK_FLAGS "-Wl,-add-stdcall-alias"
            POSITION_INDEPENDENT_CODE 0 # this is to avoid MinGW warning;
            )

    include_directories(${PROJECT_SOURCE_DIR}/vendor/libusb/include)
    include_directories(${PROJECT_SOURCE_DIR}/vendor/libsoundio/include)

elseif (MINGW)
    set(CMAKE_SYSTEM_NAME Windows)
    # set(MXEPATH /home/lukebayes/Projects/foss/mxe/usr/bin)
    set(CMAKE_C_COMPILER i686-w64-mingw32-gcc)
    set(CMAKE_CXX_COMPILER i686-w64-mingw32-g++)
    set(CMAKE_RC_COMPILER i686-w64-mingw32-windres)
    set(CMAKE_RANLIB i686-w64-mingw32-ranlib)

    set(VendorLibs
            ${VendorLibs}
            # Custom built soundio for win32 only
            # ${PROJECT_SOURCE_DIR}/vendor/libsoundio/lib/libsoundio-win32.a
            # ${PROJECT_SOURCE_DIR}/vendor/libsoundio/lib/libsoundio-win32.so
            )

    link_libraries(
            m
            pthread
            usb-1.0
            soundio
            )

    add_library(maple-sdk.dll STATIC ${AppSources} ${VendorLibs})

    set_target_properties(maple-sdk.dll
            PROPERTIES
            PREFIX ""
            SUFFIX ""
            LINK_FLAGS "-Wl,-add-stdcall-alias"
            POSITION_INDEPENDENT_CODE 0 # this is to avoid MinGW warning;
            )

    include_directories(${PROJECT_SOURCE_DIR}/vendor/libusb/include)
    include_directories(${PROJECT_SOURCE_DIR}/vendor/libsoundio/include)

    add_executable(maple.exe
            src/main_win32.c
            ${AppSources}
            ${VendorLibs}
            )
else()

    set(AppSources ${AppSources}
            src/nix/stitch.c
            )

    link_libraries(
        usb-1.0
        soundio
        m
        pthread
        )

    #find_package(SDL2_ttf REQUIRED)

    # Build non-Windows artifacts
    add_library(maple-sdk STATIC ${AppSources} ${VendorLibs})
    add_executable(maple-console src/main_console.c ${AppSources} ${VendorLibs})
    add_executable(maple-gtk src/main_gtk.c
        ${AppSources}
        ${GtkSources}
        ${VendorLibs}
        )
    add_executable(maple-test test/main_test.c
        ${AppSources}
        ${GtkSources}
        ${TestSources}
        )
    # add_executable(maple-sdl2 src/main_sdl2.c
        # src/sdl/application.c
        # src/sdl/application.h
        # )

    # Configure GTK+-3.0
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
    include_directories(${GTK3_INCLUDE_DIRS})
    link_directories(${GTK3_LIBRARY_DIRS})
    add_definitions(${GTK3_CFLAGS_OTHER})

    # Configure SDL2
    # find_package(PkgConfig REQUIRED)
    # pkg_check_modules(SDL, REQUIRED sdl2-2.0)
    # include_directories(${SDL_INCLUDE_DIRS})
    # link_directories(${SDL_LIBRARY_DIRS})
    # add_definitions(${SDL_CFLAGS_OTHER})
    # set(SdlLib sdl2-dev)
    # file(COPY ${CMAKE_SOURCE_DIR}/src/sdl/ttf DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

    # Add link relevant libraries to targets
    target_link_libraries(maple-console ${VendorLibs})
    target_link_libraries(maple-gtk ${GTK3_LIBRARIES} ${VendorLibs})
    target_link_libraries(maple-test ${GTK3_LIBRARIES} ${VendorLibs})
    # target_link_libraries(maple-sdl2 SDL2 SDL2_ttf)
    target_compile_definitions(maple-test PRIVATE TEST_MODE=1)
endif()
