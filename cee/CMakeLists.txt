cmake_minimum_required(VERSION 3.19)
project(maple C)

set(CMAKE_C_STANDARD 11)

include_directories(src)
include_directories(test)
include_directories(vendor/kissfft)

set(AppSources
        src/dtmf.c
        src/dtmf.h
        src/libusb_helper.c
        src/libusb_helper.h
        src/phony.c
        src/phony.h
        src/phony_hid.c
        src/phony_hid.h
        src/stitcher.c
        src/stitcher.h
        src/pthread_play.c
        src/pthread_play.h
        src/stitch.c
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
        test/phone_view_test.c
        test/phone_view_test.h
        test/phony_hid_test.c
        test/phony_hid_test.h
        test/phony_test.c
        test/phony_test.h
        test/stitcher_test.c
        test/stitcher_test.h
        test/test_helper.c
        test/test_helper.h
        )

set(VendorLibs
        ${PROJECT_BINARY_DIR}/../vendor/kissfft/libkissfft-float.so
        )

# link_directories(${CMAKE_BINARY_DIR}}/vendor/kissfft)

link_libraries(
        usb-1.0
        hidapi-libusb
        soundio
        m
        pthread
        )


add_executable(soundio_example src/soundio_example.c ${VendorLibs})

add_library(maple-dev STATIC ${AppSources})
add_executable(stitch_example src/stitch_example.c ${AppSources})
add_executable(maple src/main.c ${AppSources} ${VendorLibs})
add_executable(maple-test test/test_main.c
        ${AppSources}
        ${GtkSources}
        ${TestSources}
        )
add_executable(maple-gtk src/main_gtk.c
        ${AppSources}
        ${GtkSources}
        )

# Configure GTK+-3.0
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
include_directories(${GTK3_INCLUDE_DIRS})
link_directories(${GTK3_LIBRARY_DIRS})
add_definitions(${GTK3_CFLAGS_OTHER})

target_link_libraries(maple-gtk ${GTK3_LIBRARIES})
target_link_libraries(maple-test ${GTK3_LIBRARIES} ${VendorLibs})
