#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/gpio.h"
#include "tusb.h"
#include "usb_descriptors.h"

#define SCREEN_WIDTH       32767
#define SCREEN_HEIGHT      32767
#define CENTER_X           (SCREEN_WIDTH / 2)
#define CENTER_Y           (SCREEN_HEIGHT / 2)
#define RADIUS             10000

// 状态机：0 = 触摸按下并移动，1 = 触摸释放（短暂停顿）
static bool touch_active = true;
static float angle = 0.0f;
const float angle_step = 0.08f;  // 移动步长

// 发送报告（带返回值检查）
static void send_touch_report(int16_t x, int16_t y, bool touching) {
    if (!tud_hid_ready()) return;

    touch_report_t report = {
        .report_id = REPORT_ID_TOUCH,
        .tip = touching ? 1 : 0,
        .reserved1 = 0,
        .in_range = touching ? 1 : 0, // 触摸时 in_range 为真
        .confidence = 1,              // 永远相信数据有效
        .reserved2 = 0,
        .contact_id = 0,
        .x = (uint16_t)x,
        .y = (uint16_t)y
    };

    tud_hid_report(REPORT_ID_TOUCH, &report, sizeof(report));
}

int main() {
    stdio_init_all();
    tusb_init();

    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    gpio_put(PICO_DEFAULT_LED_PIN, 0);

    absolute_time_t last_report_time = get_absolute_time();
    const int64_t report_interval_us = 10000; // 10ms

    while (true) {
        tud_task(); // 处理 USB 事件

        // 只有 USB 准备好且距上次报告间隔足够时才发送新报告
        if (tud_hid_ready() && absolute_time_diff_us(last_report_time, get_absolute_time()) >= report_interval_us) {
            gpio_put(PICO_DEFAULT_LED_PIN, 1);

            int16_t x, y;
            bool send_ok = false;

            if (touch_active) {
                // 计算圆周坐标
                x = CENTER_X + (int16_t)(RADIUS * cosf(angle));
                y = CENTER_Y + (int16_t)(RADIUS * sinf(angle));

                // 边界裁剪
                if (x < 0) x = 0;
                if (x > SCREEN_WIDTH) x = SCREEN_WIDTH;
                if (y < 0) y = 0;
                if (y > SCREEN_HEIGHT) y = SCREEN_HEIGHT;

                send_ok = send_touch_report(x, y, true);

                angle += angle_step;
                if (angle >= 2.0f * M_PI) {
                    angle -= 2.0f * M_PI;
                    // 每画完一圈，模拟一次抬起（让光标短暂消失再出现，更符合真实触摸行为）
                    touch_active = false;
                }
            } else {
                // 发送释放报告（tip = 0）
                send_ok = send_touch_report(CENTER_X, CENTER_Y, false);
                touch_active = true; // 立即准备下一轮按下
            }

            if (send_ok) {
                last_report_time = get_absolute_time();
            }
        } else {
            gpio_put(PICO_DEFAULT_LED_PIN, 0);
        }

        // 极短延时，让出 CPU
        sleep_us(100);
    }

    return 0;
}
