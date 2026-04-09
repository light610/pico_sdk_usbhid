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

# 添加 src 目录作为私有包含目录（用于 tusb_config.h）
target_include_directories(${PROJECT_NAME} PRIVATE src)

# 定义 TinyUSB 配置文件宏
target_compile_definitions(${PROJECT_NAME} PRIVATE
    CFG_TUSB_CONFIG_FILE=\"tusb_config.h\"
)

# 链接 SDK 提供的 TinyUSB 设备库（自动带入头文件路径）
target_link_libraries(${PROJECT_NAME}
    pico_stdlib
    pico_unique_id
    pico_tinyusb_device   # 替换之前的 tinyusb_device
)

pico_add_extra_outputs(${PROJECT_NAME})
