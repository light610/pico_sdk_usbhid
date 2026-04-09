cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)

project(pico-touch-hid C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

# 验证 TinyUSB 子模块是否存在
if(NOT EXISTS "${PICO_SDK_PATH}/lib/tinyusb/src/tusb.h")
    message(FATAL_ERROR "TinyUSB submodule not found. Run 'git submodule update --init' in your SDK directory.")
endif()

add_executable(${PROJECT_NAME}
    src/main.c
    src/usb_descriptors.c
)

# 显式添加包含目录（这是解决问题的关键）
target_include_directories(${PROJECT_NAME} PRIVATE
    src
    ${PICO_SDK_PATH}/lib/tinyusb/src          # TinyUSB 公共头文件
)

# 自定义 tusb_config.h
target_compile_definitions(${PROJECT_NAME} PRIVATE
    CFG_TUSB_CONFIG_FILE=\"tusb_config.h\"
)

# 链接库
target_link_libraries(${PROJECT_NAME}
    pico_stdlib
    pico_unique_id
    tinyusb_device         # 提供 TinyUSB 实现
)

pico_add_extra_outputs(${PROJECT_NAME})
