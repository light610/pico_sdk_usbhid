#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "tusb.h"
#include "usb_descriptors.h"

#define SCREEN_WIDTH       32767
#define SCREEN_HEIGHT      32767
#define CENTER_X           (SCREEN_WIDTH / 2)
#define CENTER_Y           (SCREEN_HEIGHT / 2)
#define RADIUS             10000
#define UPDATE_INTERVAL_MS 10

static bool touch_active = true;
static float angle = 0.0f;
const float angle_step = 0.08f;
static uint8_t scan_time_counter = 0;

static bool send_touch_report(int16_t x, int16_t y, bool touching) {
    if (!tud_hid_ready()) return false;

    touch_report_t report = {
        .report_id = REPORT_ID_TOUCH,
        .tip = touching ? 1 : 0,
        .in_range = touching ? 1 : 0,
        .confidence = 1,
        .reserved1 = 0,
        .contact_id = 0,
        .x = (uint16_t)x,
        .y = (uint16_t)y,
        .scan_time = scan_time_counter++  // 递增扫描时间
    };

    return tud_hid_report(REPORT_ID_TOUCH, &report, sizeof(report));
}

int main() {
    stdio_init_all();
    tusb_init();

    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    gpio_put(PICO_DEFAULT_LED_PIN, 0);

    absolute_time_t last_report_time = get_absolute_time();
    const int64_t report_interval_us = UPDATE_INTERVAL_MS * 1000;

    while (true) {
        tud_task();

        if (tud_hid_ready() && absolute_time_diff_us(last_report_time, get_absolute_time()) >= report_interval_us) {
            gpio_put(PICO_DEFAULT_LED_PIN, 1);

            int16_t x, y;
            bool send_ok = false;

            if (touch_active) {
                x = CENTER_X + (int16_t)(RADIUS * cosf(angle));
                y = CENTER_Y + (int16_t)(RADIUS * sinf(angle));

                if (x < 0) x = 0;
                if (x > SCREEN_WIDTH) x = SCREEN_WIDTH;
                if (y < 0) y = 0;
                if (y > SCREEN_HEIGHT) y = SCREEN_HEIGHT;

                send_ok = send_touch_report(x, y, true);

                angle += angle_step;
                if (angle >= 2.0f * M_PI) {
                    angle -= 2.0f * M_PI;
                    touch_active = false;
                }
            } else {
                send_ok = send_touch_report(CENTER_X, CENTER_Y, false);
                touch_active = true;
            }

            if (send_ok) {
                last_report_time = get_absolute_time();
            }
        } else {
            gpio_put(PICO_DEFAULT_LED_PIN, 0);
        }

        sleep_us(100);
    }

    return 0;
}
