#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "tusb.h"
#include "usb_descriptors.h"

// 屏幕逻辑分辨率 (0 ~ 32767)
#define SCREEN_WIDTH   32767
#define SCREEN_HEIGHT  32767

// 圆心坐标
#define CENTER_X       (SCREEN_WIDTH / 2)
#define CENTER_Y       (SCREEN_HEIGHT / 2)

// 圆的半径 (逻辑单位)
#define RADIUS         10000

// 更新间隔 (毫秒)
#define UPDATE_INTERVAL_MS  10

// 发送触摸报告
static void send_touch_report(int16_t x, int16_t y, bool touching) {
    // 如果 USB 未连接或未配置，不发送
    if (!tud_hid_ready()) return;

    touch_report_t report = {
        .report_id = REPORT_ID_TOUCH,
        .contact_id = 0,
        .tip = touching ? 1 : 0,
        .reserved = 0,
        .x = (uint16_t)x,
        .y = (uint16_t)y
    };

    tud_hid_report(REPORT_ID_TOUCH, &report, sizeof(report));
}

int main() {
    // 初始化标准库
    stdio_init_all();

    // 初始化 TinyUSB
    tusb_init();

    // 可选：LED 指示
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    gpio_put(PICO_DEFAULT_LED_PIN, 0);

    // 画圆参数
    float angle = 0.0f;
    const float angle_step = 0.1f;

    while (true) {
        tud_task(); // 处理 USB 事件

        // 当 USB 挂载后，周期性发送触摸数据
        if (tud_hid_ready()) {
            gpio_put(PICO_DEFAULT_LED_PIN, 1); // 亮灯表示工作

            // 计算圆周上的点
            int16_t x = CENTER_X + (int16_t)(RADIUS * cosf(angle));
            int16_t y = CENTER_Y + (int16_t)(RADIUS * sinf(angle));

            // 限制坐标范围 (防止浮点误差越界)
            if (x < 0) x = 0;
            if (x > SCREEN_WIDTH) x = SCREEN_WIDTH;
            if (y < 0) y = 0;
            if (y > SCREEN_HEIGHT) y = SCREEN_HEIGHT;

            // 发送触摸点 (保持按下状态)
            send_touch_report(x, y, true);

            angle += angle_step;
            if (angle >= 2.0f * M_PI) angle -= 2.0f * M_PI;
        } else {
            gpio_put(PICO_DEFAULT_LED_PIN, 0);
        }

        sleep_ms(UPDATE_INTERVAL_MS);
    }

    return 0;
}
