cmake_minimum_required(VERSION 3.19)
project(maple C)

set(CMAKE_C_STANDARD 11)

include_directories(src)
include_directories(test)

add_library(maple-dev STATIC src/phony.h src/phony.c src/stitcher.c src/stitcher.h)
add_executable(maple src/main.c src/phony.h src/phony.c src/stitcher.c src/stitcher.h)
add_executable(maple-test test/test_main.c src/phony.h src/phony.c
        test/phony_test.h test/phony_test.c src/stitcher.c src/stitcher.h)

target_link_libraries(maple hidapi-libusb)
target_link_libraries(maple-dev hidapi-libusb)
target_link_libraries(maple-test hidapi-libusb)

target_link_libraries(maple soundio)
