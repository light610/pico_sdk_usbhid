cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)

project(pico-touch-hid C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(${PROJECT_NAME}
    src/main.c
    src/usb_descriptors.c
)

target_include_directories(${PROJECT_NAME} PRIVATE src)

# 启用 USB 和 TinyUSB 配置
target_compile_definitions(${PROJECT_NAME} PRIVATE
    CFG_TUSB_CONFIG_FILE=\"tusb_config.h\"
)

target_link_libraries(${PROJECT_NAME}
    pico_stdlib
    pico_unique_id
    tinyusb_device
)

pico_add_extra_outputs(${PROJECT_NAME})
