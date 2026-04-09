#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/gpio.h"
#include "tusb.h"
#include "usb_descriptors.h"

#define SCREEN_WIDTH        32767
#define SCREEN_HEIGHT       32767
#define CENTER_X            (SCREEN_WIDTH / 2)
#define CENTER_Y            (SCREEN_HEIGHT / 2)
#define RADIUS              10000       // 半径（逻辑单位）
#define SEND_INTERVAL_MS    10          // 10ms 间隔 = 100Hz

// 获取系统运行毫秒数（基于微秒计数器）
static inline uint32_t millis(void) {
    return (uint32_t)(time_us_64() / 1000);
}

// 发送触摸报告（返回值表示是否成功发送）
static bool send_touch_report(int16_t x, int16_t y, bool touching) {
    if (!tud_hid_ready()) return false;

    touch_report_t report = {
        .report_id = REPORT_ID_TOUCH,
        .contact_id = 0,
        .tip = touching ? 1 : 0,
        .reserved = 0,
        .x = (uint16_t)x,
        .y = (uint16_t)y
    };

    return tud_hid_report(REPORT_ID_TOUCH, &report, sizeof(report));
}

int main() {
    stdio_init_all();
    tusb_init();

    // 初始化 LED（用于状态指示）
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    gpio_put(PICO_DEFAULT_LED_PIN, 0);

    float angle = 0.0f;
    const float angle_step = 0.15f;       // 每次增加约 8.6 度，确保圆轨迹平滑
    uint32_t last_send_ms = 0;

    while (true) {
        tud_task();                       // 处理 USB 事件

        uint32_t now = millis();
        if (tud_hid_ready() && (now - last_send_ms >= SEND_INTERVAL_MS)) {
            // 计算圆周坐标
            int16_t x = CENTER_X + (int16_t)(RADIUS * cosf(angle));
            int16_t y = CENTER_Y + (int16_t)(RADIUS * sinf(angle));

            // 边界保护
            if (x < 0) x = 0;
            if (x > SCREEN_WIDTH) x = SCREEN_WIDTH;
            if (y < 0) y = 0;
            if (y > SCREEN_HEIGHT) y = SCREEN_HEIGHT;

            // 发送触摸按下并移动的报告（tip = true）
            if (send_touch_report(x, y, true)) {
                gpio_put(PICO_DEFAULT_LED_PIN, 1);   // 发送成功亮灯
                last_send_ms = now;
                angle += angle_step;
                if (angle >= 2.0f * M_PI) {
                    angle -= 2.0f * M_PI;
                }
            } else {
                gpio_put(PICO_DEFAULT_LED_PIN, 0);   // 发送失败灭灯
            }
        } else {
            gpio_put(PICO_DEFAULT_LED_PIN, 0);
            // 如果 USB 未连接，可以慢速等待
            sleep_ms(1);
        }
    }

    return 0;
}
