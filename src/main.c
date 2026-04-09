#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "tusb.h"
#include "usb_descriptors.h"

// 逻辑坐标范围 (与报告描述符一致)
#define LOGICAL_MAX     32767
#define LOGICAL_MIN     0

// 圆心为屏幕中央
#define CENTER_X        (LOGICAL_MAX / 2)
#define CENTER_Y        (LOGICAL_MAX / 2)

// 半径设置为逻辑范围的 1/3，在 1920x1080 屏幕上约占屏幕宽度的 1/3
#define RADIUS          (LOGICAL_MAX / 3)

// 发送间隔 (毫秒) —— 20ms 平滑且不丢包
#define REPORT_INTERVAL_MS  20

// 提前声明
static bool send_touch_report(int16_t x, int16_t y, bool touching);

int main() {
    stdio_init_all();
    tusb_init();

    // 板载 LED 指示工作状态
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    gpio_put(PICO_DEFAULT_LED_PIN, 0);

    // 等待 USB 枚举完成
    while (!tud_hid_ready()) {
        tud_task();
        sleep_ms(10);
    }

    // 初始释放一次触摸 (让系统知道当前无触摸)
    send_touch_report(CENTER_X, CENTER_Y, false);
    sleep_ms(100);

    float angle = 0.0f;
    const float angle_step = 0.15f;  // 步长，使圆更平滑

    while (true) {
        tud_task();

        if (tud_hid_ready()) {
            gpio_put(PICO_DEFAULT_LED_PIN, 1);

            // 计算圆周上的点
            int16_t x = CENTER_X + (int16_t)(RADIUS * cosf(angle));
            int16_t y = CENTER_Y + (int16_t)(RADIUS * sinf(angle));

            // 边界裁剪
            if (x < LOGICAL_MIN) x = LOGICAL_MIN;
            if (x > LOGICAL_MAX) x = LOGICAL_MAX;
            if (y < LOGICAL_MIN) y = LOGICAL_MIN;
            if (y > LOGICAL_MAX) y = LOGICAL_MAX;

            // 发送触摸按下事件
            if (send_touch_report(x, y, true)) {
                angle += angle_step;
                if (angle >= 2.0f * M_PI) angle -= 2.0f * M_PI;
            }
        } else {
            gpio_put(PICO_DEFAULT_LED_PIN, 0);
        }

        sleep_ms(REPORT_INTERVAL_MS);
    }

    return 0;
}

/**
 * 发送触摸报告，并确保报告被接受
 * 返回 true 表示发送成功，false 表示需要重试
 */
static bool send_touch_report(int16_t x, int16_t y, bool touching) {
    static uint64_t last_success_ms = 0;
    uint64_t now_ms = to_ms_since_boot(get_absolute_time());

    // 避免发送过快导致丢包
    if (now_ms - last_success_ms < (REPORT_INTERVAL_MS / 2)) {
        return false;
    }

    touch_report_t report = {
        .report_id = REPORT_ID_TOUCH,
        .contact_id = 0,
        .tip = touching ? 1 : 0,
        .reserved = 0,
        .x = (uint16_t)x,
        .y = (uint16_t)y
    };

    if (tud_hid_report(REPORT_ID_TOUCH, &report, sizeof(report))) {
        last_success_ms = now_ms;
        return true;
    }
    return false;
}
