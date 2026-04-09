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

# 添加 src 目录以包含 tusb_config.h
target_include_directories(${PROJECT_NAME} PRIVATE src)

# 使用自定义的 TinyUSB 配置文件
target_compile_definitions(${PROJECT_NAME} PRIVATE
    CFG_TUSB_CONFIG_FILE=\"tusb_config.h\"
)

# 链接必要的库（pico_tinyusb_device 会自动处理头文件路径）
target_link_libraries(${PROJECT_NAME}
    pico_stdlib
    pico_unique_id
    pico_tinyusb_device   # 注意：不是 tinyusb_device，pico_tinyusb_device 是 SDK 封装的接口库
)

pico_add_extra_outputs(${PROJECT_NAME})
