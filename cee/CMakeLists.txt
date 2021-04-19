cmake_minimum_required(VERSION 3.19)
project(maple C)

set(CMAKE_C_STANDARD 11)

include_directories(src)
include_directories(test)

add_library(maple-dev STATIC
        src/phony.h
        src/phony.c
        src/stitcher.h
        src/stitcher.c
        src/dtmf.h
        src/dtmf.c
        )
add_executable(maple
        src/main.c
        src/phony.h
        src/phony.c
        src/stitcher.h
        src/stitcher.c
        src/dtmf.h
        src/dtmf.c
        )
add_executable(maple-test
        test/test_main.c
        src/phony.h
        src/phony.c
        test/phony_test.h
        test/phony_test.c
        src/stitcher.h
        src/stitcher.c
        src/dtmf.h
        src/dtmf.c
        )

# Link the hidapi library
target_link_libraries(maple hidapi-libusb)
target_link_libraries(maple-dev hidapi-libusb)
target_link_libraries(maple-test hidapi-libusb)

# Link the soundio cross-platform audio library
target_link_libraries(maple soundio)
target_link_libraries(maple-dev soundio)
target_link_libraries(maple-test soundio)

# Link the libc Math library (m)
target_link_libraries(maple m)
target_link_libraries(maple-dev m)
target_link_libraries(maple-test m)
