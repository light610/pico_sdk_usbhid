cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)

project(pico-touch-hid C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

# 查找 tusb.h 的位置（应该位于 PICO_SDK_PATH/lib/tinyusb/src）
find_path(TINYUSB_INCLUDE_DIR
    NAMES tusb.h
    PATHS ${PICO_SDK_PATH}/lib/tinyusb/src
    NO_DEFAULT_PATH
)

if(NOT TINYUSB_INCLUDE_DIR)
    message(FATAL_ERROR "tusb.h not found in ${PICO_SDK_PATH}/lib/tinyusb/src")
endif()
message(STATUS "Found TinyUSB include dir: ${TINYUSB_INCLUDE_DIR}")

add_executable(${PROJECT_NAME}
    src/main.c
    src/usb_descriptors.c
)

target_include_directories(${PROJECT_NAME} PRIVATE
    src
    ${TINYUSB_INCLUDE_DIR}
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    CFG_TUSB_CONFIG_FILE=\"tusb_config.h\"
)

target_link_libraries(${PROJECT_NAME}
    pico_stdlib
    pico_unique_id
    tinyusb_device
)

pico_add_extra_outputs(${PROJECT_NAME})
