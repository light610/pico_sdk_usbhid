#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/gpio.h"
#include "tusb.h"
#include "usb_descriptors.h"

#define SCREEN_WIDTH   32767
#define SCREEN_HEIGHT  32767
#define CENTER_X       (SCREEN_WIDTH / 2)
#define CENTER_Y       (SCREEN_HEIGHT / 2)
#define RADIUS         12000          // 逻辑半径，视觉舒适
#define UPDATE_INTERVAL_MS  10

static void send_touch_report(int16_t x, int16_t y, bool touching) {
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
    stdio_init_all();
    tusb_init();

    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    gpio_put(PICO_DEFAULT_LED_PIN, 0);

    float angle = 0.0f;
    const float angle_step = 0.1f;

    while (true) {
        tud_task();

        if (tud_hid_ready()) {
            gpio_put(PICO_DEFAULT_LED_PIN, 1);

            int16_t x = CENTER_X + (int16_t)(RADIUS * cosf(angle));
            int16_t y = CENTER_Y + (int16_t)(RADIUS * sinf(angle));

            if (x < 0) x = 0;
            if (x > SCREEN_WIDTH) x = SCREEN_WIDTH;
            if (y < 0) y = 0;
            if (y > SCREEN_HEIGHT) y = SCREEN_HEIGHT;

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
