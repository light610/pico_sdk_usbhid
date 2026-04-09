#include "usb_descriptors.h"
#include "pico/unique_id.h"
#include "tusb.h"
#include <string.h>
#include <stdio.h>

// 设备描述符（不变）
#define USB_VID       0xCAFE
#define USB_PID       0x4005
#define USB_BCD       0x0200

static const tusb_desc_device_t device_desc = {
    .bLength            = sizeof(tusb_desc_device_t),
    .bDescriptorType    = TUSB_DESC_DEVICE,
    .bcdUSB             = USB_BCD,
    .bDeviceClass       = 0x00,
    .bDeviceSubClass    = 0x00,
    .bDeviceProtocol    = 0x00,
    .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor           = USB_VID,
    .idProduct          = USB_PID,
    .bcdDevice          = 0x0100,
    .iManufacturer      = 0x01,
    .iProduct           = 0x02,
    .iSerialNumber      = 0x03,
    .bNumConfigurations = 0x01
};

uint8_t const * tud_descriptor_device_cb(void) {
    return (uint8_t const *) &device_desc;
}

// 配置描述符
enum { ITF_NUM_HID, ITF_NUM_TOTAL };
#define EPNUM_HID   0x81
#define CONFIG_TOTAL_LEN  (TUD_CONFIG_DESC_LEN + TUD_HID_DESC_LEN)

// ***** 修正后的 HID 报告描述符 *****
static const uint8_t hid_report_desc[] = {
    0x05, 0x0D,        // Usage Page (Digitizers)
    0x09, 0x04,        // Usage (Touch Screen)
    0xA1, 0x01,        // Collection (Application)
    0x85, REPORT_ID_TOUCH, //   Report ID (1)

    // 定义物理集合 (手指)
    0x09, 0x22,        //   Usage (Finger)
    0xA1, 0x02,        //   Collection (Logical)

    // 1. Tip Switch (1 bit)
    0x09, 0x42,        //     Usage (Tip Switch)
    0x15, 0x00,        //     Logical Minimum (0)
    0x25, 0x01,        //     Logical Maximum (1)
    0x75, 0x01,        //     Report Size (1)
    0x95, 0x01,        //     Report Count (1)
    0x81, 0x02,        //     Input (Data, Var, Abs)

    // 2. 填充位 (7 bits)
    0x75, 0x01,        //     Report Size (1)
    0x95, 0x07,        //     Report Count (7)
    0x81, 0x03,        //     Input (Const, Var, Abs)

    // 3. In Range (1 bit)  ← 新增，关键！
    0x09, 0x32,        //     Usage (In Range)
    0x15, 0x00,        //     Logical Minimum (0)
    0x25, 0x01,        //     Logical Maximum (1)
    0x75, 0x01,        //     Report Size (1)
    0x95, 0x01,        //     Report Count (1)
    0x81, 0x02,        //     Input (Data, Var, Abs)

    // 4. Confidence (1 bit) ← 新增，关键！
    0x09, 0x47,        //     Usage (Confidence)
    0x15, 0x00,        //     Logical Minimum (0)
    0x25, 0x01,        //     Logical Maximum (1)
    0x75, 0x01,        //     Report Size (1)
    0x95, 0x01,        //     Report Count (1)
    0x81, 0x02,        //     Input (Data, Var, Abs)

    // 5. 填充位 (6 bits)
    0x75, 0x01,        //     Report Size (1)
    0x95, 0x06,        //     Report Count (6)
    0x81, 0x03,        //     Input (Const, Var, Abs)

    // 6. Contact ID (8 bits)
    0x09, 0x51,        //     Usage (Contact ID)
    0x15, 0x00,        //     Logical Minimum (0)
    0x25, 0x0F,        //     Logical Maximum (15)
    0x75, 0x08,        //     Report Size (8)
    0x95, 0x01,        //     Report Count (1)
    0x81, 0x02,        //     Input (Data, Var, Abs)

    // 7. X 坐标 (16 bits)
    0x05, 0x01,        //     Usage Page (Generic Desktop)
    0x09, 0x30,        //     Usage (X)
    0x15, 0x00,        //     Logical Minimum (0)
    0x26, 0xFF, 0x7F,  //     Logical Maximum (32767)
    0x75, 0x10,        //     Report Size (16)
    0x95, 0x01,        //     Report Count (1)
    0x81, 0x02,        //     Input (Data, Var, Abs)

    // 8. Y 坐标 (16 bits)
    0x09, 0x31,        //     Usage (Y)
    0x81, 0x02,        //     Input (Data, Var, Abs)

    0xC0,              //   End Collection (Logical)

    // 最大触点数量 (Feature)
    0x09, 0x55,        //   Usage (Contact Count Maximum)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x01,        //   Logical Maximum (1)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x01,        //   Report Count (1)
    0xB1, 0x02,        //   Feature (Data, Var, Abs)

    0xC0               // End Collection (Application)
};
static const uint8_t config_desc[] = {
    TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN,
                          TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 100),
    TUD_HID_DESCRIPTOR(ITF_NUM_HID, 0, HID_ITF_PROTOCOL_NONE,
                       sizeof(hid_report_desc), EPNUM_HID,
                       CFG_TUD_HID_EP_BUFSIZE, 10)
};

uint8_t const * tud_descriptor_configuration_cb(uint8_t index) {
    (void) index;
    return config_desc;
}

// 字符串描述符（不变）
static char serial_str[2 * PICO_UNIQUE_BOARD_ID_SIZE_BYTES + 1];
static const char *string_desc_arr[] = {
    (const char[]) { 0x09, 0x04 },
    "Raspberry Pi",
    "Pico Touch Screen",
    serial_str,
};

uint16_t const * tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
    static uint16_t desc_str[32];
    uint8_t len = 0;
    if (index == 0) {
        memcpy(&desc_str[1], string_desc_arr[0], 2);
        len = 1;
    } else {
        if (index >= sizeof(string_desc_arr)/sizeof(string_desc_arr[0])) return NULL;
        const char *str = string_desc_arr[index];
        if (index == 3) {
            pico_unique_board_id_t id;
            pico_get_unique_board_id(&id);
            for (int i = 0; i < PICO_UNIQUE_BOARD_ID_SIZE_BYTES; i++)
                sprintf(&serial_str[i*2], "%02X", id.id[i]);
            str = serial_str;
        }
        len = strlen(str);
        if (len > 31) len = 31;
        for (uint8_t i = 0; i < len; i++) desc_str[1+i] = str[i];
    }
    desc_str[0] = (TUSB_DESC_STRING << 8) | (2*len + 2);
    return desc_str;
}

uint8_t const * tud_hid_descriptor_report_cb(uint8_t itf) {
    (void) itf;
    return hid_report_desc;
}

// HID 回调（必须实现）
uint16_t tud_hid_get_report_cb(uint8_t itf, uint8_t report_id,
                               hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen) {
    (void) itf; (void) report_id; (void) report_type; (void) buffer; (void) reqlen;
    return 0;
}

void tud_hid_set_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type,
                           uint8_t const* buffer, uint16_t bufsize) {
    (void) itf; (void) report_id; (void) report_type; (void) buffer; (void) bufsize;
}
