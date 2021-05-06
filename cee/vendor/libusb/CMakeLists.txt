message("-- Linking libusb")
add_library(libusb "MinGW64/dll/libusb-1.0.dll")
set_target_properties(libusb PROPERTIES LINKER_LANGUAGE C)