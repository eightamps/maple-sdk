cmake_minimum_required(VERSION 3.19)
project(maple C)

set(CMAKE_C_STANDARD 11)

include_directories(src)
include_directories(test)
include_directories(vendor/kissfft)

set(AppSources
        src/dtmf.c
        src/dtmf.h
        src/phony.c
        src/phony.h
        src/phony_hid.c
        src/phony_hid.h
        src/stitch.c
        src/stitch.h
        src/hid_client.c
        src/hid_client.h
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
        )

set(VendorLibs
        ${PROJECT_SOURCE_DIR}/vendor/kissfft/libkissfft-float.so
        # Custom built soundio for win32 only
        # ${PROJECT_SOURCE_DIR}/vendor/libsoundio/lib/libsoundio-win32.so
        )

# FFT library will be used to validate generated DTMF signals
# link_directories(${CMAKE_BINARY_DIR}}/vendor/kissfft)

link_libraries(
        usb-1.0
        soundio
        hidapi-libusb
        m
        pthread
        )

# Build artifacts
add_library(maple-lib STATIC ${AppSources} ${VendorLibs})
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

# Configure GTK+-3.0
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
include_directories(${GTK3_INCLUDE_DIRS})
link_directories(${GTK3_LIBRARY_DIRS})
add_definitions(${GTK3_CFLAGS_OTHER})

# Add link relevant libraries to targets
target_link_libraries(maple-console ${VendorLibs})
target_link_libraries(maple-gtk ${GTK3_LIBRARIES} ${VendorLibs})
target_link_libraries(maple-test ${GTK3_LIBRARIES} ${VendorLibs})

target_compile_definitions(maple-test PRIVATE TEST_MODE=1)
